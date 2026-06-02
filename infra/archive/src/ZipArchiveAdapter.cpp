/**
 * @file infra/archive/src/ZipArchiveAdapter.cpp
 * @brief Implements archive packaging for exported analysis artifacts.
 */

#include "archive/pch.h"
#include "archive/ZipArchiveAdapter.h"

#include <filesystem>

#include <zip.h>

namespace infra::archive {

namespace {

bool createZipArchive(const std::filesystem::path& sourceDir,
                      const std::filesystem::path& outputArchive)
{
    int errorCode = 0;
    zip_t* archive = zip_open(outputArchive.string().c_str(), ZIP_CREATE | ZIP_TRUNCATE, &errorCode);
    if (!archive) return false;

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
            success = false;
            break;
        }

        const std::string zipPath = relativePath.generic_string();
        if (zip_file_add(archive, zipPath.c_str(), source, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8) < 0) {
            zip_source_free(source);
            success = false;
            break;
        }
    }

    if (!success) {
        zip_discard(archive);
        return false;
    }

    return zip_close(archive) == 0 && std::filesystem::exists(outputArchive);
}

} // namespace

bool ZipArchiveAdapter::create(const std::filesystem::path& sourceDirectory,
                               const std::filesystem::path& outputArchive,
                               core::ports::exporting::PackageFormat format) const
{
    if (format != core::ports::exporting::PackageFormat::Zip) {
        return false;
    }
    return createZipArchive(sourceDirectory, outputArchive);
}

} // namespace infra::archive
