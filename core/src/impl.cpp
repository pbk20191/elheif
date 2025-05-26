#include "impl.h"
// #include "libheif/heif.h"
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
#include <libheif/heif.h>

#define WRAP_ERR_RET(scope, res)                                               \
  {                                                                            \
    heif_error err = res;                                                            \
    if (err.code != heif_error_code::heif_error_Ok) {                          \
      return {                                                                 \
          .err = std::string("error when ") + scope +                          \
                 ", code: " + std::to_string(static_cast<int>(err.code)),      \
          .error = err,                                                        \
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
  return heif_error_success;
}

EncodeResult encode(const std::uint8_t* buffer, int byteSize, int width, int height) {
  
  std::vector<PixelInput> pixels;

  pixels.emplace_back(PixelInput{
    .width = static_cast<size_t>(width),
    .height = static_cast<size_t>(height),
    .data = array_view<std::uint8_t>(buffer, static_cast<size_t>(byteSize))
  });

  std::vector<uint8_t> data;
  auto writer = heif_writer {
    .writer_api_version = 1,
    .write = write_impl,
  };

  auto result = encode2(1, std::nullopt, [pixels](size_t t) { return pixels[t]; }, writer, &data);
  if (result.error.code != heif_error_Ok) {
    return {
      .err = result.err,
      .error = result.error
    };
  }
  return {.data = std::move(data)};
}

EncodeResult2 encode2(
    std::size_t frameCount,
    std::optional<EncodingOption> options,
    std::function<PixelInput(std::size_t)> getFrame,
  heif_writer& writer,
  void* userData
) {
  constexpr auto CHANNEL = heif_channel::heif_channel_interleaved;
  auto _ = CallerGuard();

  Ctx ctx;
  Encoder encoder;
  auto encoder_options = std::unique_ptr<heif_encoding_options, void (*)(heif_encoding_options*)>(
      heif_encoding_options_alloc(), heif_encoding_options_free);
  WRAP_ERR_RET("get encoder",
    heif_context_get_encoder_for_format(ctx.get(),
      heif_compression_HEVC, encoder.data()));
  ImageHandle handle;
  bool primary = true;
  std::unique_ptr<heif_color_profile_nclx, void (*)(heif_color_profile_nclx*)> nclx_profile(nullptr, nullptr);

  if (options) {
    WRAP_ERR_RET("set lossy quality",
      heif_encoder_set_lossy_quality(encoder.get(), options->quality));

    WRAP_ERR_RET("set Lossless",
      heif_encoder_set_lossless(encoder.get(), options->lossless));
    if (options->sharpYUV) {
      // encoder_options->color_conversion_options.only_use_preferred_chroma_algorithm = 1;
      encoder_options->color_conversion_options.preferred_chroma_downsampling_algorithm = heif_chroma_downsampling_sharp_yuv;
    }

    
    // if (options->lossless) {
    //     nclx_profile = std::unique_ptr<heif_color_profile_nclx, void (*)(heif_color_profile_nclx*)>(
    //       heif_nclx_color_profile_alloc(), heif_nclx_color_profile_free);
    //     WRAP_ERR_RET("set nclx profile",
    //       heif_nclx_color_profile_set_matrix_coefficients(nclx_profile.get(),
    //         heif_matrix_coefficients_RGB_GBR));


    //         // heif_nclx_profile
    //     // nclx_profile->matrix_coefficients = heif_matrix_coefficients_RGB_GBR;
    //     // heif_encoder_set
    //     encoder_options->output_nclx_profile = nclx_profile.get();
    // }
  }

  for (std::size_t i = 0; i < frameCount; ++i) {
    auto buffer = getFrame(i);
    if (buffer.data.size() == 0) {
      continue; // skip empty frames
    }
    Image img;
    const auto width = buffer.width;
    const auto height = buffer.height;
    const auto pixelBuffer = buffer.data;
    WRAP_ERR_RET("create image",
      heif_image_create(width, height, heif_colorspace_RGB,
                        heif_chroma_interleaved_RGBA, img.data()));
    WRAP_ERR_RET("add plane",
      heif_image_add_plane(img.get(), CHANNEL, width, height, 8));

      // heif_image_set_
    int stride = 0;
    auto* plane = heif_image_get_plane(img.get(), CHANNEL, &stride);
    
    // memcpy(plane, buffer, byteSize)
    if (stride == width * 4) {
      memcpy(plane, pixelBuffer.begin(), width * height * 4);
    } else {
      for (int y = 0; y < height; ++y) {
        auto dst = plane + y * stride;
        const auto src = pixelBuffer.begin() + y * width * 4;
        memcpy(dst, src, width * 4);
      }
    }

    WRAP_ERR_RET("encode image",
    heif_context_encode_image(ctx.get(), img.get(), encoder.get(), encoder_options.get(), handle.data()));
    if (primary) {
      primary = false;
      WRAP_ERR_RET(
        "Set primary Image",
        heif_context_set_primary_image(ctx.get(), handle.get())
      );
    }
  }


  WRAP_ERR_RET("write", heif_context_write(ctx.get(), &writer, userData));
  return {
    .err = "",
    .error = heif_error_success
  };
}


DecodeResult decode(const std::uint8_t *buffer, int byteSize) {
  constexpr auto CHANNEL = heif_channel::heif_channel_interleaved;
  auto _ = CallerGuard();

  Ctx ctx;
  assert(ctx.get() != nullptr);

  // WRAP_ERR_RET("init", heif_init(nullptr));
  WRAP_ERR_RET("read from memory", heif_context_read_from_memory_without_copy(
                                       ctx.get(), buffer, byteSize, nullptr));

  std::vector<ImageHandle> handles;

  auto count = heif_context_get_number_of_top_level_images(ctx.get());
  if (count > 0) {
    std::vector<heif_item_id> ids(count);
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
        .err = "",
        .error = heif_error_success,
    };
  }

  return {
      .data = std::move(bitmaps),
      .error = heif_error_success,
  };
}

} // namespace Elheif