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

emscripten::val jsDecodeImage(const std::string &buffer) {
  auto res = Elheif::decode(
      reinterpret_cast<const std::uint8_t *>(buffer.data()), buffer.size());

  auto retItems = emscripten::val::array();
  for (const auto &item : res.data) {
    auto retItem = emscripten::val::object();
    retItem.set("width", item.width);
    retItem.set("height", item.height);
    retItem.set("data", emscripten::typed_memory_view(item.data.size(),
                                                      item.data.data()));
    retItems.call<void>("push", retItem);
  }
  auto ret = emscripten::val::object();
  ret.set("err", res.err);
  ret.set("data", retItems);
  return ret;
}

emscripten::val jsEncodeImage(const std::string &buffer, int width,
                              int height) {
  auto res =
      Elheif::encode(reinterpret_cast<const std::uint8_t *>(buffer.data()),
                     buffer.size(), width, height);

  auto ret = emscripten::val::object();
  ret.set("err", res.err);
  ret.set("data",
          emscripten::typed_memory_view(res.data.size(), res.data.data()));
  return ret;
}

EMSCRIPTEN_BINDINGS() {
  function("jsDecodeImage", &jsDecodeImage);
  function("jsEncodeImage", &jsEncodeImage);
}