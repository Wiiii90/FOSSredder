/**
 * @file core/include/core/ports/infra/pdf-rendering/IPdfRenderer.h
 * @brief PDF rendering port for import workflows.
 */
#pragma once

#include "core/ports/infra/pdf-rendering/PdfRenderingRequest.h"
#include "core/ports/infra/pdf-rendering/PdfRenderingResult.h"

namespace core::ports::pdf_rendering {

/**
 * @brief Renders and extracts PDF content.
 */
class IPdfRenderer {
public:
    virtual ~IPdfRenderer() = default;

    /**
     * @brief Renders a PDF into images.
     * @param req Rendering request.
     * @return The rendering result.
     */
    virtual RenderResult render(const RenderRequest& req) = 0;

    /**
     * @brief Extracts page content from a PDF.
     * @param req Extraction request.
     * @return The extraction result.
     */
    virtual ExtractResult extract(const ExtractRequest& req) = 0;
};

} // namespace core::ports::pdf_rendering
