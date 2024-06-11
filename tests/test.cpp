
#include "impl.h"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

static std::vector<std::uint8_t> readFile(const std::string &fileName) {
  auto path = fs::path(__FILE__).parent_path() / fileName;

  std::ifstream ifs(path.c_str());
  ifs.seekg(0, std::ios::end);
  size_t fileSize = ifs.tellg();
  ifs.seekg(0);

  std::vector<std::uint8_t> bytes;
  bytes.resize(fileSize);
  ifs.read(reinterpret_cast<char *>(bytes.data()), fileSize);

  return bytes;
}

TEST(HeifTest, Decode) {
  auto buffer = readFile("example.heic");
  auto decoded = Elheif::decode(buffer.data(), buffer.size());

  // Expect equality.
  EXPECT_EQ(decoded.data.size(), 1);
  EXPECT_EQ(decoded.err, "");
  EXPECT_EQ(decoded.data[0].width, 1280);
  EXPECT_EQ(decoded.data[0].height, 720);
}

TEST(HeifTest, Encode) {
  auto buffer = readFile("example.heic");
  auto decoded = Elheif::decode(buffer.data(), buffer.size());

  auto &decodedData = decoded.data[0].data;
  auto encoded =
      Elheif::encode(decodedData.data(), decodedData.size(), 1280, 720);

  // Expect equality.
  EXPECT_EQ(encoded.err, "");
  EXPECT_EQ(encoded.data.size(), 110151);
}
