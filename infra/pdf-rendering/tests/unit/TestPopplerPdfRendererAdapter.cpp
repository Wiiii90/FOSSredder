/**
 * @file infra/pdf-rendering/tests/unit/TestPopplerPdfRendererAdapter.cpp
 * @brief Tests for the Poppler PDF renderer adapter.
 */

#include <gtest/gtest.h>

#include "core/ports/infra/pdf-rendering/IPdfRenderer.h"
#include "pdf-rendering/PopplerCore.h"
#include "unit/TestPdfSupport.h"

class IDebugger;

std::shared_ptr<core::ports::pdf_rendering::IPdfRenderer> createPdfRendererAdapter(std::shared_ptr<IDebugger> debugger);

namespace infra::pdf_rendering::tests {
namespace {

TEST(PopplerPdfRendererAdapterTest, WrapsRenderAndExtractContracts) {
    const auto tempDir = support::makeTempDir("fossredder_poppler_adapter_test");
    const auto pdfPath = support::writeSimplePdf(tempDir / "sample.pdf", "Adapter");
    auto renderer = createPdfRendererAdapter(nullptr);

    core::ports::pdf_rendering::RenderRequest renderRequest;
    renderRequest.pdfPath = pdfPath;
    renderRequest.outputDir = tempDir / "render";
    renderRequest.uniqIdPrefix = "run";
    renderRequest.filePrefix = "sample";

    const auto renderResult = renderer->render(renderRequest);
    ASSERT_EQ(renderResult.images.size(), 1u);
    EXPECT_FALSE(renderResult.imageBytes.empty());

    core::ports::pdf_rendering::ExtractRequest extractRequest;
    extractRequest.pdfPath = pdfPath;
    extractRequest.outputDir = tempDir / "meta";
    extractRequest.uniqIdPrefix = "run";
    extractRequest.filePrefix = "sample";

    const auto extractResult = renderer->extract(extractRequest);
    ASSERT_EQ(extractResult.pages.size(), 1u);
    EXPECT_FALSE(extractResult.pages.front().textElements.empty());
}

} // namespace
} // namespace infra::pdf_rendering::tests
