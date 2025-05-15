#include "impl.h"
#include "libheif/heif.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#define WRAP_ERR_RET(scope, res)                                               \
  {                                                                            \
    auto err = res;                                                            \
    if (err.code != heif_error_code::heif_error_Ok) {                          \
      return {                                                                 \
          .err = std::string("error when ") + scope +                          \
                 ", code: " + std::to_string(static_cast<int>(err.code)),      \
      };                                                                       \
    }                                                                          \
  }

namespace Elheif {

class Ctx {
public:
  Ctx() : ctx_(heif_context_alloc()) {}
  ~Ctx() { heif_context_free(ctx_); }
  heif_context *get() { return ctx_; }

private:
  heif_context *ctx_ = nullptr;
};

class CallerGuard {
public:
  CallerGuard() { heif_init(nullptr); }
  ~CallerGuard() { heif_deinit(); }
};

namespace AutoFreeWrapperPresets {

struct ImageHandle {
  using Type = heif_image_handle;
  constexpr static void free(Type *ptr) { heif_image_handle_release(ptr); }
};
struct Image {
  using Type = heif_image;
  constexpr static void free(Type *ptr) { heif_image_release(ptr); }
};
struct Encoder {
  using Type = heif_encoder;
  constexpr static void free(Type *ptr) { heif_encoder_release(ptr); }
};

} // namespace AutoFreeWrapperPresets

template <typename T> class AutoFreeWrapper {
public:
  AutoFreeWrapper() = default;
  AutoFreeWrapper(const AutoFreeWrapper<T> &rhs) = delete;
  AutoFreeWrapper(AutoFreeWrapper<T> &&rhs) {
    if (this == &rhs) {
      return;
    }
    this->ptr_ = rhs.ptr_;
    rhs.ptr_ = nullptr;
  }
  ~AutoFreeWrapper() {
    if (ptr_ != nullptr) {
      T::free(this->ptr_);
    }
  }
  typename T::Type **data() { return &ptr_; }
  typename T::Type *get() const { return ptr_; }

private:
  typename T::Type *ptr_ = nullptr;
};

using ImageHandle = AutoFreeWrapper<AutoFreeWrapperPresets::ImageHandle>;
using Image = AutoFreeWrapper<AutoFreeWrapperPresets::Image>;
using Encoder = AutoFreeWrapper<AutoFreeWrapperPresets::Encoder>;

static heif_error write_impl(heif_context *ctx, const void *data, size_t size,
                             void *userdata) {
  auto *buffer = reinterpret_cast<std::vector<std::uint8_t> *>(userdata);
  const auto *ptr = reinterpret_cast<const std::uint8_t *>(data);
  std::copy(ptr, ptr + size, std::back_inserter(*buffer));
  return heif_error{
      .code = heif_error_Ok,
      .subcode = heif_suberror_Unspecified,
      .message = "OK",
  };
}

EncodeResult encode(const std::uint8_t* buffer, int byteSize, int width, int height) {
  constexpr auto CHANNEL = heif_channel::heif_channel_interleaved;
  if (byteSize < width * height * 4) {
    return {.err = "input buffer size too small"};
  }

  auto _ = CallerGuard();
  Image img;

  WRAP_ERR_RET("create image",
    heif_image_create(width, height, heif_colorspace_RGB,
                      heif_chroma_interleaved_RGBA, img.data()));
  WRAP_ERR_RET("add plane",
    heif_image_add_plane(img.get(), CHANNEL, width, height, 8));

  int stride = 0;
  auto* plane = heif_image_get_plane(img.get(), CHANNEL, &stride);
  if (!plane) return {.err = "plane is null"};

  if (stride == width * 4) {
    memcpy(plane, buffer, width * height * 4);
  } else {
    for (int y = 0; y < height; ++y) {
      auto dst = plane + y * stride;
      auto src = buffer + y * width * 4;
      memcpy(dst, src, width * 4);
    }
  }

  Ctx ctx;
  Encoder encoder;
  WRAP_ERR_RET("get encoder",
    heif_context_get_encoder_for_format(ctx.get(),
      heif_compression_HEVC, encoder.data()));
  WRAP_ERR_RET("encode image",
    heif_context_encode_image(ctx.get(), img.get(), encoder.get(), nullptr, nullptr));

  std::vector<uint8_t> data;
  auto writer = heif_writer {
    .writer_api_version = 1,
    .write = write_impl,
  };
  WRAP_ERR_RET("write", heif_context_write(ctx.get(), &writer, &data));

  return {.data = std::move(data)};
}

DecodeResult decode(const std::uint8_t *buffer, int byteSize) {
  constexpr auto CHANNEL = heif_channel::heif_channel_interleaved;
  auto _ = CallerGuard();

  Ctx ctx;
  assert(ctx.get() != nullptr);

  // WRAP_ERR_RET("init", heif_init(nullptr));
  WRAP_ERR_RET("read from memory", heif_context_read_from_memory(
                                       ctx.get(), buffer, byteSize, nullptr));

  std::vector<ImageHandle> handles;

  {
    ImageHandle primaryHandle;
    WRAP_ERR_RET("get primary handle", heif_context_get_primary_image_handle(
                                           ctx.get(), primaryHandle.data()));
    handles.emplace_back(std::move(primaryHandle));
  }

  auto count = heif_context_get_number_of_top_level_images(ctx.get());
  if (count > 0) {
    std::vector<heif_item_id> ids;
    auto received = heif_context_get_list_of_top_level_image_IDs(
        ctx.get(), ids.data(), count);
    for (const auto id : ids) {
      ImageHandle handle;
      WRAP_ERR_RET("get image handle",
                   heif_context_get_image_handle(ctx.get(), id, handle.data()));
      handles.emplace_back(std::move(handle));
    }
  }

  std::vector<Bitmap> bitmaps;
  for (const auto &handle : handles) {
    Image img;
    WRAP_ERR_RET("decode image",
                 heif_decode_image(handle.get(), img.data(),
                                   heif_colorspace::heif_colorspace_RGB,
                                   heif_chroma::heif_chroma_interleaved_RGBA,
                                   nullptr));

    int stride;
    auto *plane = heif_image_get_plane_readonly(img.get(), CHANNEL, &stride);

    auto width = heif_image_get_width(img.get(), CHANNEL);
    auto height = heif_image_get_height(img.get(), CHANNEL);
    std::vector<std::uint8_t> data;

    if (stride == width * 4) {
      data.insert(data.end(), plane, plane + width * height * 4);
    } else {
      for (int y = 0; y < height; y++) {
        const auto* row = plane + y * stride;
        data.insert(data.end(), row, row + width * 4);
      }
    }

    // std::copy(plane, plane + stride * height, std::back_inserter(data));
    bitmaps.emplace_back(Bitmap{
        .width = width,
        .height = height,
        .data = std::move(data),
    });
  }

  if (bitmaps.empty()) {
    return {
        .err = "empty images",
    };
  }

  return {
      .data = std::move(bitmaps),
  };
}

} // namespace Elheif