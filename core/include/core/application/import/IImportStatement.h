/**
 * @file core/include/core/application/import/IImportStatement.h
 * @brief Declares the public statement import service interface and factory.
 */

#pragma once

#include "core/application/import/ImportRequest.h"
#include "core/application/import/ImportResult.h"
#include "core/errors/IErrorReporter.h"

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

/**
 * @brief Public statement import service used by the import job orchestration.
 */
class IImportStatement {
public:
    /**
     * @brief Destroy the import service interface.
     */
    virtual ~IImportStatement() = default;

    /**
     * @brief Execute the import workflow.
     * @param req Import execution request.
     * @return Import workflow result.
     */
    virtual ImportResult importStatement(const ImportRequest& req) = 0;
};

/**
 * @brief Creates the default statement import service over the configured adapters.
 * @param poppler PDF rendering adapter.
 * @param openCv Image processing adapter.
 * @param tesseract Text recognition adapter.
 * @param errorReporter Optional error reporter.
 * @return Shared import service instance.
 */
std::shared_ptr<IImportStatement> createImportStatement(std::shared_ptr<core::ports::pdf_rendering::IPdfRenderer> poppler,
                                                        std::shared_ptr<core::ports::image_processing::IImageProcessor> openCv,
                                                        std::shared_ptr<core::ports::text_recognition::ITextRecognizer> tesseract,
                                                        std::shared_ptr<core::errors::IErrorReporter> errorReporter = nullptr);

}
