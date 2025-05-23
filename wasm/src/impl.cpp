#include "impl.h"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

emscripten::val vector_to_uint8array(const std::vector<uint8_t> &buffer)
{
  using namespace emscripten;

  val uint8array = val::global("Uint8Array").new_(buffer.size());
  val view = val(typed_memory_view(buffer.size(), buffer.data()));
  uint8array.call<void>("set", view);

  return uint8array;
}
#include <assert.h>
emscripten::val jsDecodeImage(const std::string &buffer)
{
  using namespace emscripten;

  auto res = Elheif::decode(
      reinterpret_cast<const std::uint8_t *>(buffer.data()), buffer.size());

  auto retItems = emscripten::val::array();
  const val ImageData = emscripten::val::global("ImageData");
  for (const auto &item : res.data)
  {
    auto imageData = ImageData.new_(item.width, item.height);
    auto view = typed_memory_view(item.data.size(), item.data.data());
    auto backing = imageData["data"];
    imageData["data"].call<void>("set", view);
    retItems.call<void>("push", imageData);
  }
  auto ret = emscripten::val::object();
  ret.set("err", res.err);
  ret.set("data", retItems);
  return ret;
}

std::vector<uint8_t> toVector(const emscripten::val& jsArray) {
    const auto length = jsArray["byteLength"].as<unsigned>();
    std::vector<uint8_t> vec(length);
    emscripten::val memoryView = emscripten::val(emscripten::typed_memory_view<uint8_t>(length, vec.data()));
    memoryView.call<void>("set", jsArray);
    return vec;
}

emscripten::val jsEncodeImage(const emscripten::val &imageData)
{
  const size_t width = imageData["width"].as<size_t>();
  const size_t height = imageData["height"].as<size_t>();
  const emscripten::val jsPixelBuffer = imageData["data"];
  const auto pixelBuffer = jsPixelBuffer.as<std::string>();
  auto res =
      Elheif::encode(reinterpret_cast<const std::uint8_t *>(pixelBuffer.data()),
                     pixelBuffer.size(), width, height);

  auto ret = emscripten::val::object();
  ret.set("err", res.err);
  ret.set("data", vector_to_uint8array(res.data));
  return ret;
}

EMSCRIPTEN_BINDINGS()
{
  function("jsDecodeImage", &jsDecodeImage, emscripten::return_value_policy::take_ownership());
  function("jsEncodeImage", &jsEncodeImage), emscripten::return_value_policy::take_ownership();
}