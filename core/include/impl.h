#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <libheif/heif.h>
namespace Elheif {


template<typename T>
class array_view {
   const T* ptr_;
   std::size_t len_;
public:
   array_view(const T* ptr, std::size_t len) noexcept: ptr_{ptr}, len_{len} {}

   T& operator[](int i) noexcept { return ptr_[i]; }
   T const& operator[](int i) const noexcept { return ptr_[i]; }
   auto size() const noexcept { return len_; }

   auto begin() const noexcept { return ptr_; }
   auto end() const noexcept { return ptr_ + len_; }
};

struct PixelInput {
  std::size_t width;
  std::size_t height;
  array_view<uint8_t> data;
};

struct Bitmap {
  int width;
  int height;
  std::vector<std::uint8_t> data;
};

struct DecodeResult {
  std::string err;
  std::vector<Bitmap> data;
  heif_error error = heif_error_success;
};

struct EncodeResult {
  std::string err = "";
  std::vector<std::uint8_t> data;
  heif_error error = heif_error_success;
};

struct EncodeResult2 {
  std::string err = "";
  heif_error error = heif_error_success;
};

DecodeResult decode(const std::uint8_t *buffer, int byteSize);
EncodeResult encode(const std::uint8_t *buffer, int byteSize, int width,
                    int height);

EncodeResult2 encode2(
  const std::vector<PixelInput> & pixels,

  heif_writer& writer,
  void* userData
);

} // namespace Elheif