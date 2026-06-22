/**
 * @file core/include/core/pch.h
 * @brief Collects common core includes and aliases for precompiled-header builds.
 */

#pragma once

#define NOMINMAX

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "core/application/export/ExportLog.h"
#include "core/application/import/ImportLog.h"
#include "core/application/import/draft/StatementDraft.h"
#include "core/application/import/draft/TransactionDraft.h"
#include "core/application/storage/DeletionImpact.h"
#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/domain/entities/Actor.h"
#include "core/domain/entities/Analysis.h"
#include "core/domain/entities/Annual.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Property.h"
#include "core/domain/entities/Statement.h"
#include "core/domain/entities/Transaction.h"

using core::application::exporting::ExportLog;
using core::application::importing::ImportLog;
using core::application::importing::draft::StatementDraft;
using core::application::importing::draft::TransactionDraft;
using core::domain::Actor;
using core::domain::Analysis;
using core::domain::Annual;
using core::domain::Contract;
using core::domain::DeletionImpact;
using core::domain::Property;
using core::domain::Statement;
using core::domain::Transaction;
using core::domain::catalog::WorkspaceCatalog;
