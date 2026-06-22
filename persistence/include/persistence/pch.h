#pragma once

#define NOMINMAX

#include <chrono>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "core/domain/catalog/WorkspaceCatalog.h"
#include "core/domain/entities/Actor.h"
#include "core/domain/entities/Analysis.h"
#include "core/domain/entities/Annual.h"
#include "core/domain/entities/Contract.h"
#include "core/domain/entities/Property.h"
#include "core/domain/entities/Statement.h"
#include "core/domain/entities/Transaction.h"

using core::domain::Actor;
using core::domain::Analysis;
using core::domain::Annual;
using core::domain::Contract;
using core::domain::Property;
using core::domain::Statement;
using core::domain::Transaction;
using core::domain::catalog::WorkspaceCatalog;
