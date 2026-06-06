/**
 * @file core/include/core/ports/infra/pdf-rendering/PdfRenderingResult.h
 * @brief Result types for PDF rendering operations.
 */
#pragma once

#include "core/ports/infra/pdf-rendering/PdfRenderingTypes.h"

#include <filesystem>
#include <vector>

namespace core::ports::pdf_rendering {

struct RenderResult {
    std::vector<std::filesystem::path> images;
    std::vector<std::vector<uint8_t>> imageBytes;
};

struct ExtractResult {
    std::vector<RenderedPage> pages;
};

} // namespace core::ports::pdf_rendering
