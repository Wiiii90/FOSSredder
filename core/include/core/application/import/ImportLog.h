/**
 * @file core/include/core/application/import/ImportLog.h
 * @brief Declares the persisted metadata record for an import run.
 */

#pragma once

#include <string>
#include <vector>

namespace core::application::importing {

/**
 * @brief Stores metadata about one import run for persistence and workflow tracking.
 */
class ImportLog {
public:
    std::string id;
    std::string time;
    std::string type;
    std::string file;
    std::string status;
    std::string message;
    bool draftAttached = false;
    std::string draftId;
    std::vector<std::string> statementDraftIds;
    std::string statementId;
};

}
