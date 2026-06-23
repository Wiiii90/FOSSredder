/**
 * @file infra/pdf-rendering/tests/unit/TestPopplerCore.cpp
 * @brief Tests for the Poppler PDF rendering helpers.
 */

#include <gtest/gtest.h>

#include "pdf-rendering/PopplerCore.h"
#include "unit/TestPdfSupport.h"

#include <stdexcept>

namespace infra::pdf_rendering::tests {
namespace {

TEST(PopplerCoreTest, RendersAndExtractsSinglePagePdf) {
    const auto tempDir = support::makeTempDir("fossredder_poppler_core_test");
    const auto pdfPath = support::writeSimplePdf(tempDir / "sample.pdf", "Hello PDF");
    const auto renderDir = tempDir / "render";
    const auto metaDir = tempDir / "meta";

    const auto pages = PopplerCore::renderDocument(pdfPath.string(), 72.0, renderDir, "run", "sample");
    ASSERT_EQ(pages.size(), 1u);
    EXPECT_FALSE(pages.front().imageBytes.empty());
    EXPECT_TRUE(std::filesystem::exists(pages.front().imagePath));
    EXPECT_GT(pages.front().imageWidthPx, 0);
    EXPECT_GT(pages.front().imageHeightPx, 0);

    const auto meta = PopplerCore::extractDocumentMeta(pdfPath.string(), 72.0, metaDir, "run", "sample");
    ASSERT_EQ(meta.size(), 1u);
    EXPECT_FALSE(meta.front().metadataJson.empty());
    EXPECT_FALSE(meta.front().textElements.empty());
    EXPECT_GT(meta.front().pageWidthPts, 0.0);
    EXPECT_GT(meta.front().pageHeightPts, 0.0);
}

TEST(PopplerCoreTest, ThrowsForMissingPdf) {
    EXPECT_THROW((PopplerCore::renderDocument("P:/missing.pdf", 72.0)), std::runtime_error);
    EXPECT_THROW((PopplerCore::extractDocumentMeta("P:/missing.pdf", 72.0)), std::runtime_error);
}

} // namespace
} // namespace infra::pdf_rendering::tests
