/**
 * @file infra/analysis-image-renderer/tests/unit/TestOpenCvAnalysisImageRendererAdapter.cpp
 * @brief Tests for the OpenCV analysis image renderer adapter.
 */

#include <gtest/gtest.h>

#include "analysis-image-renderer/OpenCvAnalysisImageRendererAdapter.h"
#include "core/constants/analysis.h"
#include "core/ports/diagnostics/IErrorReporter.h"

#include <cmath>
#include <memory>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core.hpp>

namespace infra::analysis_image_renderer::tests {
namespace {

class NoopErrorReporter final
    : public core::ports::diagnostics::IErrorReporter {
public:
    void report(const core::errors::ErrorEvent&) override {}
};

std::shared_ptr<core::ports::diagnostics::IErrorReporter> makeErrorReporter()
{
    return std::make_shared<NoopErrorReporter>();
}

std::filesystem::path makeTempDir(const std::string& stem) {
    auto dir = std::filesystem::temp_directory_path() / stem;
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);
    return dir;
}

class OpenCvAnalysisImageRendererAdapterTest : public ::testing::TestWithParam<std::string> {
protected:
    static core::ports::analysis::AnalysisResult makeResult(const std::string& type) {
        core::ports::analysis::AnalysisResult result;
        result.type = type;
        if (type == core::constants::analysis::plotTypes::kPie) {
            result.table = {
                {"Rent", "2.0"},
                {"Food", "1.0"},
            };
        } else if (type == core::constants::analysis::plotTypes::kHistogram) {
            result.table = {
                {"January", R"({"month":"January","total":120.5})"},
                {"February", R"({"month":"February","total":98.0})"},
            };
        } else {
            result.type = core::constants::analysis::kTypeCalculation;
            result.table = {
                {"Adjustment", R"({"amountOriginal":100.0,"amountAdjusted":80.0,"taxPercent":19.0,"taxFactor":0.19,"transactionId":"tx-1"})"},
                {"Refund", R"({"amountOriginal":25.0,"amountAdjusted":25.0,"taxPercent":0.0,"taxFactor":0.0,"transactionId":"tx-2"})"},
            };
        }
        return result;
    }
};

TEST_P(OpenCvAnalysisImageRendererAdapterTest, WritesRenderableImageForSupportedAnalysisTypes) {
    const auto tempDir = ::infra::analysis_image_renderer::tests::makeTempDir("fossredder_analysis_image_renderer_test");
    const auto outputPath = tempDir / (GetParam() + ".png");
    const auto result = makeResult(GetParam());

    OpenCvAnalysisImageRendererAdapter adapter(makeErrorReporter());
    ASSERT_TRUE(adapter.writeAnalysisImage(outputPath, "Analysis", result));
    ASSERT_TRUE(std::filesystem::exists(outputPath));
    ASSERT_GT(std::filesystem::file_size(outputPath), 0u);

    const auto image = cv::imread(outputPath.string(), cv::IMREAD_UNCHANGED);
    ASSERT_FALSE(image.empty());
}

TEST(OpenCvAnalysisImageRendererAdapterStandaloneTest, PieUsesDistinctAdjacentSliceColors)
{
    const auto tempDir = makeTempDir("fossredder_analysis_image_renderer_palette_test");
    const auto outputPath = tempDir / "pie_palette.png";

    core::ports::analysis::AnalysisResult result;
    result.type = core::constants::analysis::plotTypes::kPie;
    result.table = {
        {"A", "1.0"},
        {"B", "1.0"},
        {"C", "1.0"},
    };

    OpenCvAnalysisImageRendererAdapter adapter(makeErrorReporter());
    ASSERT_TRUE(adapter.writeAnalysisImage(outputPath, "Analysis", result));

    const auto image = cv::imread(outputPath.string(), cv::IMREAD_UNCHANGED);
    ASSERT_FALSE(image.empty());
    ASSERT_GE(image.channels(), 3);

    const int width = image.cols;
    const int height = image.rows;
    const double scale = std::max(1.0, static_cast<double>(width) / 1280.0);
    const int margin = static_cast<int>(std::round(24.0 * scale));
    const int legendWidth = std::max(320, width / 3);
    const int pieAreaWidth = width - legendWidth - (margin * 3);
    const int radiusPx = std::max(static_cast<int>(std::round(120.0 * scale)),
                                  std::min(pieAreaWidth / 2, height / 2 - margin));
    const int cx = margin + radiusPx;
    const int cy = margin + radiusPx;
    const double sampleR = static_cast<double>(radiusPx) * 0.6;

    auto sampleAt = [&](double degrees) {
        const double radians = degrees * (3.14159265358979323846 / 180.0);
        const int x = static_cast<int>(std::round(cx + std::cos(radians) * sampleR));
        const int y = static_cast<int>(std::round(cy + std::sin(radians) * sampleR));
        return image.at<cv::Vec4b>(y, x);
    };

    // With 3 equal slices and startAngle=-90, the sector centers are -30, 90, and 210 degrees.
    const cv::Vec4b c1 = sampleAt(-30.0);
    const cv::Vec4b c2 = sampleAt(90.0);
    const cv::Vec4b c3 = sampleAt(210.0);

    EXPECT_NE(c1, c2);
    EXPECT_NE(c2, c3);
    EXPECT_NE(c1, c3);
}

INSTANTIATE_TEST_SUITE_P(
    SupportedTypes,
    OpenCvAnalysisImageRendererAdapterTest,
    ::testing::Values(
        std::string(core::constants::analysis::plotTypes::kPie),
        std::string(core::constants::analysis::plotTypes::kHistogram),
        std::string(core::constants::analysis::kTypeCalculation)));

} // namespace
} // namespace infra::analysis_image_renderer::tests
