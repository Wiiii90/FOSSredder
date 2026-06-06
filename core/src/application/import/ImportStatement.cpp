/**
 * @file core/src/application/import/ImportStatement.cpp
 * @brief Factory and thin implementation for IImportStatement using strategies.
 */

#include "core/pch.h"
#include "core/application/import/IImportStatement.h"
#include "ImportStatementStrategy.h"

#include <memory>

namespace core::application::importing {

namespace {

class ImportStatementImpl final : public IImportStatement {
public:
    explicit ImportStatementImpl(std::unique_ptr<IImportStatementStrategy> strategy)
        : strategy_(std::move(strategy)) {}

    ImportResult importStatement(const ImportRequest& req) override {
        if (!strategy_) return {};
        return strategy_->run(req);
    }

private:
    std::unique_ptr<IImportStatementStrategy> strategy_;
};

}

std::shared_ptr<IImportStatement> createImportStatement(std::shared_ptr<core::ports::pdf_rendering::IPdfRenderer> poppler,
                                                        std::shared_ptr<core::ports::image_processing::IImageProcessor> openCv,
                                                        std::shared_ptr<core::ports::text_recognition::ITextRecognizer> tesseract,
                                                        std::shared_ptr<core::errors::IErrorReporter> errorReporter) {
    auto strat = createDefaultImportStrategy(poppler, openCv, tesseract, std::move(errorReporter));
    return std::make_shared<ImportStatementImpl>(std::move(strat));
}

}
