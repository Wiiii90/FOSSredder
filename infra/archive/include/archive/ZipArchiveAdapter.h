/**
 * @file infra/archive/include/archive/ZipArchiveAdapter.h
 * @brief Declares the archive adapter used by export packaging.
 */

#pragma once

#include "core/ports/infra/archive/IArchive.h"

#include <filesystem>
#include <memory>

namespace core::errors {
class IErrorReporter;
}

namespace infra::archive {

/**
 * @brief Creates ZIP archives for exported analysis artifacts.
 */
class ZipArchiveAdapter final : public core::ports::archive::IArchive {
public:
    explicit ZipArchiveAdapter(std::shared_ptr<core::errors::IErrorReporter> errorReporter = nullptr);

    /**
     * @brief Creates a ZIP archive from the files contained in a source directory.
     * @param sourceDirectory The directory whose files should be packaged.
     * @param outputArchive The target archive file path.
     * @param format The requested archive format.
     * @return True when the archive was created successfully.
     */
    bool create(const std::filesystem::path& sourceDirectory,
                const std::filesystem::path& outputArchive,
                core::ports::exporting::PackageFormat format) const override;

private:
    std::shared_ptr<core::errors::IErrorReporter> errorReporter_;
};

} // namespace infra::archive
