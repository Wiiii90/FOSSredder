/**
 * @file core/include/core/ports/infra/archive/IArchive.h
 * @brief Declares the archive abstraction used by export packaging.
 */

#pragma once

#include "core/ports/usecases/export/ExportRequest.h"

#include <filesystem>

namespace core::ports::archive {

/**
 * @brief Abstraction for writing exported artifacts into archive files.
 */
class IArchive {
public:
    /**
     * @brief Destroy the archive abstraction.
     */
    virtual ~IArchive() = default;

    /**
     * @brief Creates an archive from files inside a source directory.
     * @param sourceDirectory Directory containing the files to package.
     * @param outputArchive Target archive file path.
     * @param format Requested archive format.
     * @return True when the archive was created successfully.
     */
    virtual bool create(const std::filesystem::path& sourceDirectory,
                        const std::filesystem::path& outputArchive,
                        core::ports::exporting::PackageFormat format) const = 0;
};

} // namespace core::ports::archive
