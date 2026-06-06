/**
 * @file infra/image-processing/tests/unit/TestMaskAdapter.cpp
 * @brief Tests for the OpenCV masking adapter.
 */

#include <gtest/gtest.h>

#include "image-processing/MaskAdapter.h"
#include "unit/TestImageProcessingSupport.h"

#include <opencv2/imgcodecs.hpp>

namespace infra::image_processing::tests {
namespace {

TEST(MaskAdapterTest, BuildsMaskAndWhiteoutFromProvidedBoxes) {
    auto image = support::makeCanvas(160, 120);
    support::drawTextBlock(image, cv::Rect(20, 20, 50, 18));
    support::drawTextBlock(image, cv::Rect(95, 40, 45, 20));

    const auto tempDir = support::makeTempDir("fossredder_mask_adapter_test");
    const core::ports::image_processing::MaskRequest request{
        {},
        support::encodePng(image),
        tempDir,
        "mask",
        "whiteout",
        {
            {20, 20, 50, 18},
            {95, 40, 45, 20},
        },
        core::ports::image_processing::MaskRequest::Mode::Whiteout,
        {},
        false,
        false,
        false,
        {}
    };

    const auto result = opencv::MaskAdapter::mask(request);

    ASSERT_FALSE(result.maskImageBytes.empty());
    ASSERT_FALSE(result.maskedImageBytes.empty());
    EXPECT_TRUE(std::filesystem::exists(result.maskedImagePath));

    const auto maskImage = cv::imdecode(result.maskImageBytes, cv::IMREAD_GRAYSCALE);
    const auto maskedImage = cv::imdecode(result.maskedImageBytes, cv::IMREAD_COLOR);
    ASSERT_FALSE(maskImage.empty());
    ASSERT_FALSE(maskedImage.empty());
    EXPECT_GT(cv::countNonZero(maskImage), 0);
    EXPECT_EQ(maskedImage.cols, image.cols);
    EXPECT_EQ(maskedImage.rows, image.rows);
}

TEST(MaskAdapterTest, ReturnsEmptyResultForMissingInput) {
    const core::ports::image_processing::MaskRequest request{
        "P:/does/not/exist.png",
        {},
        {},
        {},
        {},
        {},
        core::ports::image_processing::MaskRequest::Mode::Whiteout,
        {},
        false,
        false,
        false,
        {}
    };

    const auto result = opencv::MaskAdapter::mask(request);

    EXPECT_TRUE(result.maskImageBytes.empty());
    EXPECT_TRUE(result.maskedImageBytes.empty());
    EXPECT_TRUE(result.maskedImagePath.empty());
}

TEST(MaskAdapterTest, CreatesLineMasksForHorizontalAndVerticalStructures) {
    cv::Mat image = support::makeCanvas(120, 80);
    cv::line(image, cv::Point(10, 20), cv::Point(110, 20), cv::Scalar(0, 0, 0), 2);
    cv::line(image, cv::Point(30, 10), cv::Point(30, 70), cv::Scalar(0, 0, 0), 2);

    const auto horizontal = opencv::MaskAdapter::makeLineMask(image, true);
    const auto vertical = opencv::MaskAdapter::makeLineMask(image, false);

    ASSERT_FALSE(horizontal.empty());
    ASSERT_FALSE(vertical.empty());
    EXPECT_GT(cv::countNonZero(horizontal), 0);
    EXPECT_GT(cv::countNonZero(vertical), 0);
}

} // namespace
} // namespace infra::image_processing::tests
