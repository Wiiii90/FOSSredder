/**
 * @file infra/image-processing/tests/unit/TestDetectAdapter.cpp
 * @brief Tests for the OpenCV detection adapter.
 */

#include <gtest/gtest.h>

#include "image-processing/DetectAdapter.h"
#include "unit/TestImageProcessingSupport.h"

namespace infra::image_processing::tests {
namespace {

TEST(DetectAdapterTest, ReturnsEmptyResultsForBlankImages) {
    const auto image = support::makeCanvas(160, 120);

    const auto textBlocks = opencv::DetectAdapter::detectTextBlocks(image);
    const auto tables = opencv::DetectAdapter::detectTables(image, "blank.png");

    EXPECT_TRUE(textBlocks.empty());
    EXPECT_TRUE(tables.empty());
}

TEST(DetectAdapterTest, DetectsTextBlocksOnSolidDarkRegions) {
    auto image = support::makeCanvas(240, 160);
    support::drawTextBlock(image, cv::Rect(20, 20, 140, 22));
    support::drawTextBlock(image, cv::Rect(30, 70, 160, 18));

    const auto textBlocks = opencv::DetectAdapter::detectTextBlocks(image);

    ASSERT_FALSE(textBlocks.empty());
    EXPECT_GE(textBlocks.size(), 1u);
    const auto& first = textBlocks.front();
    EXPECT_GT(first.width, 50);
    EXPECT_GT(first.height, 10);
}

TEST(DetectAdapterTest, DetectsTablesOnSimpleGridImages) {
    auto image = support::makeCanvas(320, 240);
    support::drawGrid(image, 3, 3, 18, 3);

    const auto tables = opencv::DetectAdapter::detectTables(image, "grid.png");

    ASSERT_FALSE(tables.empty());
    const auto& table = tables.front();
    EXPECT_GT(table.cells.size(), 0u);
    EXPECT_GT(table.rows, 0);
    EXPECT_GT(table.cols, 0);
    EXPECT_GT(table.bbox.width, 0);
    EXPECT_GT(table.bbox.height, 0);
}

} // namespace
} // namespace infra::image_processing::tests
