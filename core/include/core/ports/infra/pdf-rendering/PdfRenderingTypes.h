/**
 * @file core/include/core/ports/infra/pdf-rendering/PdfRenderingTypes.h
 * @brief Shared PDF rendering data types.
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace core::ports::pdf_rendering {

struct TextElement {
    std::string text;
    double x = 0.0;
    double y = 0.0;
    double width = 0.0;
    double height = 0.0;
};

struct RenderedPage {
    std::string imagePath;
    std::vector<uint8_t> imageBytes;
    int imageWidthPx = 0;
    int imageHeightPx = 0;
    std::string metadataJson;
    double pageWidthPts = 0.0;
    double pageHeightPts = 0.0;
    double dpiX = 0.0;
    double dpiY = 0.0;
    std::vector<TextElement> textElements;
};

} // namespace core::ports::pdf_rendering
