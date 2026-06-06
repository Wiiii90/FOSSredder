/**
 * @file core/src/application/import/ImportStatementStrategy.h
 * @brief Declares private import strategy contracts used only by core import implementation files.
 */

#pragma once

#include "core/application/import/IImportStatement.h"

#include <memory>

namespace core::ports::pdf_rendering {
class IPdfRenderer;
}
namespace core::ports::image_processing {
class IImageProcessor;
}
namespace core::ports::text_recognition {
class ITextRecognizer;
}

namespace core::application::importing {

class IImportStatementStrategy {
public:
    virtual ~IImportStatementStrategy() = default;
    virtual ImportResult run(const ImportRequest& req) = 0;
};

std::unique_ptr<IImportStatementStrategy> createDefaultImportStrategy(std::shared_ptr<core::ports::pdf_rendering::IPdfRenderer> poppler,
                                                                      std::shared_ptr<core::ports::image_processing::IImageProcessor> opencv,
                                                                      std::shared_ptr<core::ports::text_recognition::ITextRecognizer> tesseract,
                                                                      std::shared_ptr<core::errors::IErrorReporter> errorReporter = nullptr);

}
