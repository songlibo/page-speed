/**
 * Copyright 2009 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Author: Bryan McQuade, Matthew Steele

#include <fstream>
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "pagespeed/image_compression/jpeg_optimizer.h"
#include "pagespeed/testing/pagespeed_test.h"

namespace {

using pagespeed::image_compression::OptimizeJpeg;
using pagespeed::image_compression::OptimizeJpegWithOptions;

// The JPEG_TEST_DIR_PATH macro is set by the gyp target that builds this file.
const std::string kJpegTestDir = IMAGE_TEST_DIR_PATH "jpeg/";

struct ImageCompressionInfo {
  const char* filename;
  size_t original_size;
  size_t compressed_size;
  size_t lossy_compressed_size;
  size_t progressive_size;
  size_t progressive_and_lossy_compressed_size;
};

ImageCompressionInfo kValidImages[] = {
  { "sjpeg1.jpg", 1552, 1536, 1165, 1774, 1410 },
  { "sjpeg2.jpg", 3612, 3283, 3630, 3475, 3798 },
  { "sjpeg3.jpg", 44084, 41664, 26924, 40997, 25814 },
  { "sjpeg4.jpg", 168895, 168240, 51389, 162867, 49186 },
  { "sjpeg6.jpg", 149600, 147163, 89671, 146038, 84641 },
  { "test411.jpg", 6883, 4367, 3709, 4540, 3854 },
  { "test420.jpg", 6173, 3657, 3653, 3796, 3793 },
  { "test422.jpg", 6501, 3985, 3712, 4152, 3849 },
  { "testgray.jpg", 5014, 3072, 3060, 3094, 3078 },
};

const char *kInvalidFiles[] = {
  "notajpeg.png",  // A png.
  "notajpeg.gif",  // A gif.
  "emptyfile.jpg", // A zero-byte file.
  "corrupt.jpg",   // Invalid huffman code in the image data section.
};

// Given one of the above file names, read the contents of the file into the
// given destination string.

void ReadJpegToString(const std::string &file_name, std::string *dest) {
  const std::string path = kJpegTestDir + file_name;
  pagespeed_testing::ReadFileToString(path, dest);
}

void WriteStringToFile(const std::string &file_name, std::string &src) {
  const std::string path = kJpegTestDir + file_name;
  std::ofstream stream;
  stream.open(path.c_str(), std::ofstream::out | std::ofstream::binary);
  stream.write(src.c_str(), src.size());
  stream.close();
}

const size_t kValidImageCount = arraysize(kValidImages);
const size_t kInvalidFileCount = arraysize(kInvalidFiles);

TEST(JpegOptimizerTest, ValidJpegs) {
  for (size_t i = 0; i < kValidImageCount; ++i) {
    std::string src_data;
    ReadJpegToString(kValidImages[i].filename, &src_data);
    std::string dest_data;
    ASSERT_TRUE(OptimizeJpeg(src_data, &dest_data));
    EXPECT_EQ(kValidImages[i].original_size, src_data.size())
        << kValidImages[i].filename;
    EXPECT_EQ(kValidImages[i].compressed_size, dest_data.size())
        << kValidImages[i].filename;

    // Uncomment this next line for debugging:
    //WriteStringToFile(std::string("z") + kValidImages[i].filename, dest_data);

    ASSERT_LE(dest_data.size(), src_data.size());
  }
}

TEST(JpegOptimizerTest, ValidJpegsLossy) {
  for (size_t i = 0; i < kValidImageCount; ++i) {
    std::string src_data;
    ReadJpegToString(kValidImages[i].filename, &src_data);
    pagespeed::image_compression::JpegCompressionOptions options;
    options.lossy = true;
    options.quality = 85;
    std::string dest_data;
    ASSERT_TRUE(OptimizeJpegWithOptions(src_data, &dest_data, &options))
        << kValidImages[i].filename;
    EXPECT_EQ(kValidImages[i].original_size, src_data.size())
        << kValidImages[i].filename;
    EXPECT_EQ(kValidImages[i].lossy_compressed_size, dest_data.size())
        << kValidImages[i].filename;

    // Uncomment this next line for debugging:
    //WriteStringToFile(std::string("l") + kValidImages[i].filename, dest_data);
  }
}

TEST(JpegOptimizerTest, ValidJpegsProgressive) {
  for (size_t i = 0; i < kValidImageCount; ++i) {
    std::string src_data;
    ReadJpegToString(kValidImages[i].filename, &src_data);
    pagespeed::image_compression::JpegCompressionOptions options;
    options.progressive = true;
    std::string dest_data;
    ASSERT_TRUE(OptimizeJpegWithOptions(src_data, &dest_data, &options))
        << kValidImages[i].filename;
    EXPECT_EQ(kValidImages[i].original_size, src_data.size())
        << kValidImages[i].filename;
    EXPECT_EQ(kValidImages[i].progressive_size, dest_data.size())
        << kValidImages[i].filename;

    // Uncomment this next line for debugging:
    //WriteStringToFile(std::string("p") + kValidImages[i].filename, dest_data);
  }
}

TEST(JpegOptimizerTest, ValidJpegsProgressiveAndLossy) {
  for (size_t i = 0; i < kValidImageCount; ++i) {
    std::string src_data;
    ReadJpegToString(kValidImages[i].filename, &src_data);
    pagespeed::image_compression::JpegCompressionOptions options;
    options.lossy = true;
    options.quality = 85;
    options.progressive = true;
    std::string dest_data;
    ASSERT_TRUE(OptimizeJpegWithOptions(src_data, &dest_data, &options))
        << kValidImages[i].filename;
    EXPECT_EQ(kValidImages[i].original_size, src_data.size())
        << kValidImages[i].filename;
    EXPECT_EQ(kValidImages[i].progressive_and_lossy_compressed_size,
              dest_data.size()) << kValidImages[i].filename;

    // Uncomment this next line for debugging:
    //WriteStringToFile(std::string("b") + kValidImages[i].filename, dest_data);
  }
}

TEST(JpegOptimizerTest, InvalidJpegs) {
  for (size_t i = 0; i < kInvalidFileCount; ++i) {
    std::string src_data;
    ReadJpegToString(kInvalidFiles[i], &src_data);
    std::string dest_data;
    ASSERT_FALSE(OptimizeJpeg(src_data, &dest_data));
  }
}

TEST(JpegOptimizerTest, InvalidJpegsLossy) {
  for (size_t i = 0; i < kInvalidFileCount; ++i) {
    std::string src_data;
    ReadJpegToString(kInvalidFiles[i], &src_data);
    pagespeed::image_compression::JpegCompressionOptions options;
    options.lossy = true;
    options.quality = 85;
    std::string dest_data;
    ASSERT_FALSE(OptimizeJpegWithOptions(src_data, &dest_data, &options));
  }
}

TEST(JpegOptimizerTest, InvalidJpegsProgressive) {
  for (size_t i = 0; i < kInvalidFileCount; ++i) {
    std::string src_data;
    ReadJpegToString(kInvalidFiles[i], &src_data);
    pagespeed::image_compression::JpegCompressionOptions options;
    options.progressive = true;
    std::string dest_data;
    ASSERT_FALSE(OptimizeJpegWithOptions(src_data, &dest_data, &options));
  }
}

TEST(JpegOptimizerTest, InvalidJpegsProgressiveAndLossy) {
  for (size_t i = 0; i < kInvalidFileCount; ++i) {
    std::string src_data;
    ReadJpegToString(kInvalidFiles[i], &src_data);
    pagespeed::image_compression::JpegCompressionOptions options;
    options.lossy = true;
    options.quality = 85;
    options.progressive = true;
    std::string dest_data;
    ASSERT_FALSE(OptimizeJpegWithOptions(src_data, &dest_data, &options));
  }
}

// Test that after reading an invalid jpeg, the reader cleans its state so that
// it can read a correct jpeg again.
TEST(JpegOptimizerTest, CleanupAfterReadingInvalidJpeg) {
  // Compress each input image with a reinitialized JpegOptimizer.
  // We will compare these files with the output we get from
  // a JpegOptimizer that had an error.
  std::vector<std::string> correctly_compressed;
  for (size_t i = 0; i < kValidImageCount; ++i) {
    std::string src_data;
    ReadJpegToString(kValidImages[i].filename, &src_data);
    correctly_compressed.push_back("");
    std::string &dest_data = correctly_compressed.back();
    ASSERT_TRUE(OptimizeJpeg(src_data, &dest_data));
  }

  // The invalid files are all invalid in different ways, and we want to cover
  // all the ways jpeg decoding can fail.  So, we want at least as many valid
  // images as invalid ones.
  ASSERT_GE(kValidImageCount, kInvalidFileCount);

  for (size_t i = 0; i < kInvalidFileCount; ++i) {
    std::string invalid_src_data;
    ReadJpegToString(kInvalidFiles[i], &invalid_src_data);
    std::string invalid_dest_data;

    std::string valid_src_data;
    ReadJpegToString(kValidImages[i].filename, &valid_src_data);
    std::string valid_dest_data;

    ASSERT_FALSE(OptimizeJpeg(invalid_src_data, &invalid_dest_data));
    ASSERT_TRUE(OptimizeJpeg(valid_src_data, &valid_dest_data));

    // Diff the jpeg created by CreateOptimizedJpeg() with the one created
    // with a reinitialized JpegOptimizer.
    ASSERT_EQ(valid_dest_data, correctly_compressed.at(i));
  }
}

}  // namespace
