/**
 * @file ui/src/workspace/WorkspaceCache.cpp
 * @brief Owns workspace cache models, filters, and data revision lifecycle.
 */

#include "ui/workspace/WorkspaceCache.h"

#include <cmath>
#include <cstddef>
#include <optional>
#include <utility>

#include <QMetaType>
#include <QSet>

#include "ui/shared/util/StringConversions.h"
#include "ui/workspace/TransactionFilterModel.h"

namespace ui {

namespace {

template <typename IdRange> QSet<QString> toQStringSet(const IdRange &ids) {
  QSet<QString> values;
  values.reserve(static_cast<qsizetype>(ids.size()));
  for (const auto &id : ids) {
    values.insert(QString::fromStdString(id));
  }
  return values;
}

std::vector<std::string> toStdStringVector(const QStringList &ids) {
  std::vector<std::string> values;
  values.reserve(static_cast<size_t>(ids.size()));
  for (const auto &id : ids) {
    values.push_back(strings::toStdString(id));
  }
  return values;
}

std::optional<double> parseManualAmountInput(const QString &text) {
  QString raw = text.trimmed();
  raw.remove(QChar(0x00A0));
  raw.remove(QChar(0x202F));
  raw.remove(QLatin1Char(' '));
  if (raw.isEmpty()) {
    return std::nullopt;
  }

  bool okDirect = false;
  const double direct = raw.toDouble(&okDirect);
  if (okDirect && std::isfinite(direct)) {
    return direct;
  }

  int decimalPos = -1;
  for (int i = raw.size() - 1; i >= 0; --i) {
    const QChar ch = raw.at(i);
    if (ch == QLatin1Char('.') || ch == QLatin1Char(',')) {
      decimalPos = i;
      break;
    }
  }

  QString canonical;
  canonical.reserve(raw.size());
  for (int i = 0; i < raw.size(); ++i) {
    const QChar ch = raw.at(i);
    if (ch.isDigit()) {
      canonical.append(ch);
      continue;
    }
    if ((ch == QLatin1Char('-') || ch == QLatin1Char('+')) && i == 0) {
      canonical.append(ch);
      continue;
    }
    if (ch == QLatin1Char('.') || ch == QLatin1Char(',')) {
      if (i == decimalPos) {
        canonical.append(QLatin1Char('.'));
      }
      continue;
    }
    return std::nullopt;
  }

  if (canonical.isEmpty() || canonical == QLatin1String("-") ||
      canonical == QLatin1String("+")) {
    return std::nullopt;
  }

  bool okCanonical = false;
  const double parsed = canonical.toDouble(&okCanonical);
  if (!okCanonical || !std::isfinite(parsed)) {
    return std::nullopt;
  }
  return parsed;
}

template <typename IdRange, typename Model, typename Cleanup>
void removeDeletedIds(const IdRange &ids, Model &model, Cleanup &&cleanup) {
  for (const auto &rawId : ids) {
    const QString id = QString::fromStdString(rawId);
    const int row = model.findRowById(id);
    if (row >= 0) {
      model.removeAt(row);
    }
    cleanup(id);
  }
}

template <typename IdRange, typename FindRow, typename RemoveAt,
          typename Cleanup>
void removeDeletedIds(const IdRange &ids, FindRow &&findRow,
                      RemoveAt &&removeAt, Cleanup &&cleanup) {
  for (const auto &rawId : ids) {
    const QString id = QString::fromStdString(rawId);
    const int row = findRow(id);
    if (row >= 0) {
      removeAt(row);
    }
    cleanup(id);
  }
}

} // namespace

WorkspaceCache::WorkspaceCache(QObject *parent)
    : QObject(parent), filters_(this), models_(this) {}

void WorkspaceCache::bumpDataRevision() {
  ++dataRevision_;
  emit dataRevisionChanged();
}

void WorkspaceCache::loadFromState(
    const core::ports::workspace::WorkspaceSnapshot &state) {
  filters_.clear();
  models_.loadFromState(state);
  bumpDataRevision();
}

TransactionFilter *
WorkspaceCache::statementTransactions(const QString &statementId) {
  return filters_.statementTransactions(statementId, models_.transactions());
}

TransactionFilter *
WorkspaceCache::propertyTransactions(const QString &propertyId) {
  return filters_.propertyTransactions(propertyId, models_.transactions());
}

void WorkspaceCache::applyDeletionImpact(
    const core::ports::workspace::DeletionImpact &impact) {
  removeDeletedIds(impact.deletedTransactionIds, models_.transactions(),
                   [](const QString &) {});

  removeDeletedIds(
      impact.deletedStatementIds, models_.statements(),
      [this](const QString &id) { filters_.removeStatement(id); });

  removeDeletedIds(
      impact.deletedPropertyIds,
      [this](const QString &id) { return models_.findPropertyRowById(id); },
      [this](int row) { models_.removePropertyAt(row); },
      [this](const QString &id) { filters_.removeProperty(id); });

  removeDeletedIds(
      impact.deletedActorIds,
      [this](const QString &id) { return models_.findActorRowById(id); },
      [this](int row) { models_.removeActorAt(row); }, [](const QString &) {});
  removeDeletedIds(
      impact.deletedContractIds,
      [this](const QString &id) { return models_.findContractRowById(id); },
      [this](int row) { models_.removeContractAt(row); },
      [](const QString &) {});

  removeDeletedIds(impact.deletedAnalysisIds, models_.analyses(),
                   [](const QString &) {});
  removeDeletedIds(impact.deletedAnnualIds, models_.annuals(),
                   [](const QString &) {});

  models_.refreshContractTypes();
  bumpDataRevision();
}

void WorkspaceCache::setTransactionPropertyIdsImmediate(
    const QString &txId, const QStringList &propertyIds) {
  if (txId.isEmpty()) {
    return;
  }
  const int row = models_.transactions().findRowById(txId);
  if (row < 0) {
    return;
  }

  auto current =
      models_.transactions().transactions().at(static_cast<size_t>(row));
  QSet<QString> oldSet = toQStringSet(current.propertyIds);
  const QSet<QString> newSet(propertyIds.begin(), propertyIds.end());

  if (oldSet == newSet) {
    return;
  }

  current.propertyIds = toStdStringVector(propertyIds);
  models_.transactions().setTransactionAt(row, std::move(current));
  bumpDataRevision();
}

double WorkspaceCache::amountForTransactionCommit(
    const QVariant &rawAmount, const QString &transactionId,
    double fallbackAmount) const {
  if (rawAmount.userType() == QMetaType::Double) {
    const double numeric = rawAmount.toDouble();
    if (std::isfinite(numeric)) {
      return numeric;
    }
  }

  if (const auto flexible = parseManualAmountInput(rawAmount.toString())) {
    return *flexible;
  }

  if (!transactionId.isEmpty()) {
    const int row = models_.transactions().findRowById(transactionId);
    if (row >= 0) {
      const auto &rows = models_.transactions().transactions();
      const auto &tx = rows.at(static_cast<std::size_t>(row));
      const double persistedAmount = tx.amount;
      if (std::isfinite(persistedAmount)) {
        return persistedAmount;
      }
    }
  }

  return std::isfinite(fallbackAmount) ? fallbackAmount : 0.0;
}

} // namespace ui
