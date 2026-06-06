/**
 * @file infra/image-processing/tests/unit/TestDenoiseAdapter.cpp
 * @brief Tests for the OpenCV denoise adapter.
 */

#include <gtest/gtest.h>

#include "image-processing/DenoiseAdapter.h"
#include "unit/TestImageProcessingSupport.h"

namespace infra::image_processing::tests {
namespace {

class DenoiseAdapterTest : public ::testing::TestWithParam<core::ports::image_processing::DenoiseRequest::Method> {
protected:
    std::filesystem::path tempDir_;
    std::filesystem::path inputPath_;

    void SetUp() override {
        tempDir_ = support::makeTempDir("fossredder_denoise_adapter_test");
        const auto image = support::makeCanvas(120, 80);
        inputPath_ = support::writePng(tempDir_ / "input.png", image);
    }
};

TEST_P(DenoiseAdapterTest, WritesDenoisedImageForEachSupportedMethod) {
    const auto method = GetParam();
    const core::ports::image_processing::DenoiseRequest request{
        inputPath_,
        method,
        {}
    };

    const auto result = opencv::DenoiseAdapter::denoise(request);

    ASSERT_FALSE(result.denoisedImagePath.empty());
    EXPECT_TRUE(std::filesystem::exists(result.denoisedImagePath));
    EXPECT_EQ(result.denoisedImagePath.extension().string(), ".png");
}

TEST(DenoiseAdapterTest, ReturnsEmptyResultForMissingFile) {
    const auto tempDir = support::makeTempDir("fossredder_denoise_adapter_missing");
    const core::ports::image_processing::DenoiseRequest request{
        tempDir / "missing.png",
        core::ports::image_processing::DenoiseRequest::Method::Median,
        {}
    };

    const auto result = opencv::DenoiseAdapter::denoise(request);

    EXPECT_TRUE(result.denoisedImagePath.empty());
}

INSTANTIATE_TEST_SUITE_P(
    Methods,
    DenoiseAdapterTest,
    ::testing::Values(
        core::ports::image_processing::DenoiseRequest::Method::Median,
        core::ports::image_processing::DenoiseRequest::Method::Gaussian,
        core::ports::image_processing::DenoiseRequest::Method::Bilateral));

} // namespace
} // namespace infra::image_processing::tests
