/**
 * @file infra/image-processing/tests/unit/TestCropAdapter.cpp
 * @brief Tests for the OpenCV crop adapter.
 */

#include <gtest/gtest.h>

#include "image-processing/CropAdapter.h"
#include "unit/TestImageProcessingSupport.h"

#include <opencv2/imgcodecs.hpp>

namespace infra::image_processing::tests {
namespace {

TEST(CropAdapterTest, CropsInMemoryImageAndSkipsInvalidRectangles) {
    auto image = support::makeCanvas(120, 80);
    support::drawTextBlock(image, cv::Rect(10, 10, 40, 20));
    support::drawTextBlock(image, cv::Rect(70, 30, 30, 20));

    const auto tempDir = support::makeTempDir("fossredder_crop_adapter_test");
    std::vector<std::vector<uint8_t>> bytes;
    const std::vector<core::ports::image_processing::Rect> rects = {
        {10, 10, 40, 20},
        {70, 30, 80, 40},
        {5, 5, 0, 0},
    };

    const auto paths = opencv::CropAdapter::cropImages(
        image,
        rects,
        tempDir,
        core::ports::image_processing::CropRequest::OutputFormat::Png,
        92,
        &bytes,
        nullptr,
        "crop");

    ASSERT_EQ(paths.size(), 2u);
    ASSERT_EQ(bytes.size(), 2u);
    EXPECT_TRUE(std::filesystem::exists(paths[0]));
    EXPECT_TRUE(std::filesystem::exists(paths[1]));
    EXPECT_FALSE(bytes[0].empty());
    EXPECT_FALSE(bytes[1].empty());

    const auto first = cv::imdecode(bytes[0], cv::IMREAD_COLOR);
    const auto second = cv::imdecode(bytes[1], cv::IMREAD_COLOR);
    ASSERT_FALSE(first.empty());
    ASSERT_FALSE(second.empty());
    EXPECT_EQ(first.cols, 40);
    EXPECT_EQ(first.rows, 20);
    EXPECT_GT(second.cols, 0);
    EXPECT_GT(second.rows, 0);
}

TEST(CropAdapterTest, WritesJpgFilesWhenRequested) {
    auto image = support::makeCanvas(90, 60);
    support::drawTextBlock(image, cv::Rect(15, 15, 30, 20));

    const auto tempDir = support::makeTempDir("fossredder_crop_adapter_jpg_test");
    const std::vector<core::ports::image_processing::Rect> rects = {
        {15, 15, 30, 20},
    };

    const auto paths = opencv::CropAdapter::cropImages(
        image,
        rects,
        tempDir,
        core::ports::image_processing::CropRequest::OutputFormat::Jpg,
        85);

    ASSERT_EQ(paths.size(), 1u);
    EXPECT_EQ(paths.front().extension().string(), ".jpg");
    EXPECT_TRUE(std::filesystem::exists(paths.front()));
    EXPECT_GT(std::filesystem::file_size(paths.front()), 0u);
}

} // namespace
} // namespace infra::image_processing::tests
