/**
 * @file infra/archive/src/ZipArchiveAdapter.cpp
 * @brief Implements archive packaging for exported analysis artifacts.
 */

#include "archive/pch.h"
#include "archive/ZipArchiveAdapter.h"

#include "core/errors/ErrorCodes.h"
#include "core/errors/ErrorReporting.h"

#include <exception>
#include <filesystem>
#include <string>
#include <utility>

#include <zip.h>

namespace infra::archive {

namespace {

void reportArchiveFailure(core::errors::IErrorReporter* reporter,
                          std::string message,
                          core::errors::ErrorContext context = {})
{
    if (!reporter) return;

    core::errors::ErrorEvent event;
    event.severity = core::errors::ErrorSeverity::Error;
    event.origin = "infra::archive::ZipArchiveAdapter::create";
    event.message = std::move(message);
    event.code = core::errors::codes::GenericError;
    event.context = std::move(context);
    core::errors::report(reporter, event);
}

bool createZipArchive(const std::filesystem::path& sourceDir,
                      const std::filesystem::path& outputArchive,
                      core::errors::IErrorReporter* errorReporter)
{
    int errorCode = 0;
    zip_t* archive = zip_open(outputArchive.string().c_str(), ZIP_CREATE | ZIP_TRUNCATE, &errorCode);
    if (!archive) {
        reportArchiveFailure(errorReporter,
                             "failed to open output archive",
                             {{"path", outputArchive.string()}, {"zipErrorCode", std::to_string(errorCode)}});
        return false;
    }

    bool success = true;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(sourceDir)) {
        if (!entry.is_regular_file()) continue;

        const std::filesystem::path absolutePath = entry.path();
        std::filesystem::path relativePath;
        try {
            relativePath = std::filesystem::relative(absolutePath, sourceDir);
        } catch (...) {
            relativePath = absolutePath.filename();
        }

        zip_source_t* source = zip_source_file(archive, absolutePath.string().c_str(), 0, 0);
        if (!source) {
            reportArchiveFailure(errorReporter,
                                 "failed to create zip source",
                                 {{"path", absolutePath.string()}});
            success = false;
            break;
        }

        const std::string zipPath = relativePath.generic_string();
        if (zip_file_add(archive, zipPath.c_str(), source, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8) < 0) {
            zip_source_free(source);
            reportArchiveFailure(errorReporter,
                                 "failed to add file to zip archive",
                                 {{"path", absolutePath.string()}, {"archivePath", zipPath}});
            success = false;
            break;
        }
    }

    if (!success) {
        zip_discard(archive);
        return false;
    }

    const bool closed = zip_close(archive) == 0;
    const bool exists = std::filesystem::exists(outputArchive);
    if (!closed || !exists) {
        reportArchiveFailure(errorReporter,
                             "failed to finalize zip archive",
                             {{"path", outputArchive.string()}});
    }
    return closed && exists;
}

} // namespace

ZipArchiveAdapter::ZipArchiveAdapter(std::shared_ptr<core::errors::IErrorReporter> errorReporter)
    : errorReporter_(std::move(errorReporter))
{
}

bool ZipArchiveAdapter::create(const std::filesystem::path& sourceDirectory,
                               const std::filesystem::path& outputArchive,
                               core::ports::exporting::PackageFormat format) const
{
    if (format != core::ports::exporting::PackageFormat::Zip) {
        reportArchiveFailure(errorReporter_.get(),
                             "unsupported archive format",
                             {{"path", outputArchive.string()}});
        return false;
    }
    try {
        return createZipArchive(sourceDirectory, outputArchive, errorReporter_.get());
    } catch (...) {
        core::errors::reportException(errorReporter_.get(),
                                      core::errors::ErrorSeverity::Error,
                                      "infra::archive::ZipArchiveAdapter::create",
                                      std::current_exception());
        return false;
    }
}

} // namespace infra::archive
