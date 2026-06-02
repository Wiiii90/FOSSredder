/**
 * @file ui/src/workspace/RowSelectionSupport.cpp
 * @brief Implements shared QML row selection and ordering helpers.
 */

#include "ui/workspace/RowSelectionSupport.h"

#include <algorithm>

#include "ui/shared/payload/PayloadKeys.h"

namespace ui {

namespace {

QVariantMap selectionStateMap(int currentIndex, const QString &selectedId,
                              const QString &idKey) {
  QVariantMap out;
  out.insert(QStringLiteral("index"), currentIndex);
  out.insert(QStringLiteral("id"), selectedId);
  out.insert(QStringLiteral("idKey"), idKey);
  return out;
}

QString rowIdAt(const QVariantList &rows, int index, const QString &idKey) {
  if (rows.isEmpty() || index < 0 || index >= rows.size()) {
    return {};
  }
  return rows.at(index).toMap().value(idKey).toString();
}

int rowIndexOfId(const QVariantList &rows, const QString &id,
                 const QString &idKey) {
  if (id.isEmpty()) {
    return -1;
  }
  for (int i = 0; i < rows.size(); ++i) {
    const QVariantMap row = rows.at(i).toMap();
    if (row.value(idKey).toString() == id) {
      return i;
    }
  }
  return -1;
}

QVariantList pruneAndAppendMissingIds(const QVariantList &preferredIds,
                                      const QVariantList &availableIds) {
  QVariantList out;
  for (const auto &id : preferredIds) {
    if (availableIds.contains(id) && !out.contains(id)) {
      out.push_back(id);
    }
  }
  for (const auto &id : availableIds) {
    if (!out.contains(id)) {
      out.push_back(id);
    }
  }
  return out;
}

int selectionIndexOfId(const QVariantList &rows, const QString &id,
                       const QString &idKey) {
  return rowIndexOfId(rows, id, idKey);
}

int selectionIndexOfKeyValue(const QVariantList &rows, const QString &key,
                             const QVariant &value) {
  for (int i = 0; i < rows.size(); ++i) {
    if (rows.at(i).toMap().value(key) == value) {
      return i;
    }
  }
  return -1;
}

int normalizedSelectionIndex(int index, int count) {
  if (count <= 0) {
    return -1;
  }
  return std::clamp(index, 0, count - 1);
}

int wrappedSelectionIndex(int index, int count) {
  if (count <= 0) {
    return -1;
  }
  const int normalized = index % count;
  return normalized < 0 ? normalized + count : normalized;
}

QString wrappedSelectionIdAt(const QVariantList &rows, int index,
                             const QString &idKey) {
  const int wrapped = wrappedSelectionIndex(index, rows.size());
  if (wrapped < 0) {
    return {};
  }
  return rowIdAt(rows, wrapped, idKey);
}

QString navigatedSelectionIdForRows(const QVariantList &rows,
                                    const QString &currentId, int delta,
                                    int defaultIndex, const QString &idKey) {
  const int currentIndex = selectionIndexOfId(rows, currentId, idKey);
  if (currentIndex < 0) {
    if (rows.isEmpty()) {
      return {};
    }
    if (delta > 0) {
      return rowIdAt(rows, 0, idKey);
    }
    if (delta < 0) {
      return rowIdAt(rows, rows.size() - 1, idKey);
    }
    return rowIdAt(rows, defaultIndex, idKey);
  }
  if (delta > 0) {
    if (currentIndex >= rows.size() - 1) {
      return {};
    }
    return rowIdAt(rows, currentIndex + 1, idKey);
  }
  if (delta < 0) {
    if (currentIndex <= 0) {
      return {};
    }
    return rowIdAt(rows, currentIndex - 1, idKey);
  }
  return rowIdAt(rows, currentIndex, idKey);
}

QVariantList selectionRowIds(const QVariantList &rows, const QString &idKey) {
  QVariantList out;
  out.reserve(rows.size());
  for (const auto &rowValue : rows) {
    const auto row = rowValue.toMap();
    const auto id = row.value(idKey);
    if (!id.isNull() && !id.toString().isEmpty()) {
      out.push_back(id);
    }
  }
  return out;
}

QVariantList orderedRowsBySelectionIds(const QVariantList &rows,
                                       const QVariantList &orderIds,
                                       const QString &idKey) {
  QVariantList out;
  out.reserve(rows.size());
  for (const auto &orderId : orderIds) {
    const auto orderKey = orderId.toString();
    for (const auto &rowValue : rows) {
      const auto row = rowValue.toMap();
      if (row.value(idKey).toString() == orderKey && !out.contains(rowValue)) {
        out.push_back(rowValue);
        break;
      }
    }
  }
  for (const auto &rowValue : rows) {
    if (!out.contains(rowValue)) {
      out.push_back(rowValue);
    }
  }
  return out;
}

QVariantMap resolveSelectionRowState(const QVariantList &rows, int currentIndex,
                                     const QString &selectedId,
                                     const QString &idKey) {
  const int selectedIndex = rowIndexOfId(rows, selectedId, idKey);
  const int resolvedIndex =
      selectedIndex >= 0 ? selectedIndex
                         : normalizedSelectionIndex(currentIndex, rows.size());
  const QString resolvedId =
      !selectedId.isEmpty() ? selectedId : rowIdAt(rows, resolvedIndex, idKey);

  QVariantMap out = selectionStateMap(resolvedIndex, resolvedId, idKey);
  out.insert(QStringLiteral("currentId"), rowIdAt(rows, resolvedIndex, idKey));
  return out;
}

QVariantList orderWithInsertedSelectionId(const QVariantList &currentOrder,
                                          const QVariantList &availableIds,
                                          const QString &insertedId,
                                          int insertAfterIndex) {
  QVariantList out = pruneAndAppendMissingIds(currentOrder, availableIds);
  if (!insertedId.isEmpty() && !out.contains(insertedId)) {
    const int insertIndex =
        std::clamp(insertAfterIndex + 1, 0, static_cast<int>(out.size()));
    out.insert(insertIndex, insertedId);
  }
  return out;
}

QVariantMap orderedSelectionRowsState(const QVariantList &rows,
                                      const QVariantList &preferredOrder,
                                      const QString &idKey) {
  const QVariantList orderedRows =
      orderedRowsBySelectionIds(rows, preferredOrder, idKey);
  const QVariantList orderIds =
      pruneAndAppendMissingIds(preferredOrder, selectionRowIds(rows, idKey));

  QVariantMap out;
  out.insert(QStringLiteral("rows"), orderedRows);
  out.insert(QStringLiteral("orderIds"), orderIds);
  out.insert(QStringLiteral("order"), orderIds);
  return out;
}

QVariantMap orderedSelectionStateForRows(const QVariantList &rows,
                                         const QVariantList &preferredOrder,
                                         int currentIndex,
                                         const QString &selectedId,
                                         const QString &idKey) {
  QVariantMap out = orderedSelectionRowsState(rows, preferredOrder, idKey);
  const QVariantList orderedRows = out.value(QStringLiteral("rows")).toList();
  const QVariantMap selection =
      resolveSelectionRowState(orderedRows, currentIndex, selectedId, idKey);
  out.insert(QStringLiteral("selection"), selection);
  out.insert(QStringLiteral("index"), selection.value(QStringLiteral("index")));
  out.insert(QStringLiteral("id"), selection.value(QStringLiteral("id")));
  out.insert(QStringLiteral("currentId"),
             selection.value(QStringLiteral("currentId")));
  out.insert(QStringLiteral("idKey"), selection.value(QStringLiteral("idKey")));
  return out;
}

QString deleteNextSelectionIdForRows(const QVariantList &rows,
                                     const QString &removedId, int defaultIndex,
                                     const QString &idKey) {
  const int removedIndex = selectionIndexOfId(rows, removedId, idKey);
  const int nextIndex = removedIndex >= 0 ? removedIndex + 1 : defaultIndex;
  return wrappedSelectionIdAt(rows, nextIndex, idKey);
}

QVariantMap navigateSelectionDeltaState(const QVariantList &rows,
                                        int currentIndex,
                                        const QString &selectedId, int delta,
                                        int defaultIndex,
                                        const QString &idKey) {
  QVariantMap out =
      resolveSelectionRowState(rows, currentIndex, selectedId, idKey);
  const int currentRowIndex = rowIndexOfId(rows, selectedId, idKey);
  const int baseIndex = currentRowIndex >= 0 ? currentRowIndex : defaultIndex;
  out.insert(QStringLiteral("id"),
             wrappedSelectionIdAt(rows, baseIndex + delta, idKey));
  out.insert(QStringLiteral("index"),
             wrappedSelectionIndex(baseIndex + delta, rows.size()));
  return out;
}

QVariantMap deleteReselectionStateForRows(const QVariantList &rows,
                                          const QVariantList &preferredOrder,
                                          int currentIndex,
                                          const QString &removedId,
                                          const QString &idKey) {
  QVariantList remaining = rows;
  for (int i = 0; i < remaining.size(); ++i) {
    if (remaining.at(i).toMap().value(idKey).toString() == removedId) {
      remaining.removeAt(i);
      break;
    }
  }
  QVariantMap out = orderedSelectionStateForRows(remaining, preferredOrder,
                                                 currentIndex, {}, idKey);
  out.insert(
      QStringLiteral("nextId"),
      deleteNextSelectionIdForRows(rows, removedId, currentIndex, idKey));
  return out;
}

} // namespace

int indexOfId(const QVariantList &rows, const QString &id) {
  return selectionIndexOfId(rows, id, ui::payload::keys::common::kId);
}

int indexOfKeyValue(const QVariantList &rows, const QString &key,
                    const QVariant &value) {
  return selectionIndexOfKeyValue(rows, key, value);
}

int wrappedIndex(int index, int count) {
  return wrappedSelectionIndex(index, count);
}

QString navigatedSelectionId(const QVariantList &rows, const QString &currentId,
                             int delta, int defaultIndex,
                             const QString &idKey) {
  return navigatedSelectionIdForRows(rows, currentId, delta, defaultIndex,
                                     idKey);
}

QVariantList displayRowsWithEmpty(const QVariantList &rows,
                                  const QString &emptyDisplay,
                                  const QString &displayKey) {
  QVariantList out;

  QVariantMap empty;
  empty.insert(ui::payload::keys::common::kId, QString());
  empty.insert(ui::payload::keys::common::kDisplay, emptyDisplay);
  out.push_back(empty);

  for (const auto &rowValue : rows) {
    const QVariantMap row = rowValue.toMap();
    QVariantMap displayRow;
    displayRow.insert(ui::payload::keys::common::kId,
                      row.value(ui::payload::keys::common::kId).toString());

    QString display = row.value(displayKey).toString();
    if (display.isEmpty()) {
      display = row.value(ui::payload::keys::common::kDisplay).toString();
    }
    if (display.isEmpty()) {
      display = row.value(ui::payload::keys::common::kName).toString();
    }
    displayRow.insert(ui::payload::keys::common::kDisplay, display);
    out.push_back(displayRow);
  }

  return out;
}

QVariantList rowIds(const QVariantList &rows, const QString &idKey) {
  return selectionRowIds(rows, idKey);
}

bool rowHasId(const QVariantList &rows, const QString &id,
              const QString &idKey) {
  const QString target = id.trimmed();
  if (target.isEmpty()) {
    return false;
  }

  for (const QVariant &rowValue : rows) {
    const QVariantMap row = rowValue.toMap();
    if (row.value(idKey).toString() == target) {
      return true;
    }
  }
  return false;
}

QVariantMap rowById(const QVariantList &rows, const QString &id,
                    const QString &idKey) {
  const int index = rowIndexOfId(rows, id.trimmed(), idKey);
  return index >= 0 ? rows.at(index).toMap() : QVariantMap{};
}

QString rememberedOrFirstRowId(const QVariantList &rows,
                               const QVariantMap &rememberedIds,
                               const QString &ownerId, const QString &idKey) {
  const QString remembered =
      rememberedIds.value(ownerId.trimmed()).toString().trimmed();
  if (rowHasId(rows, remembered, idKey)) {
    return remembered;
  }
  if (rows.isEmpty()) {
    return QString();
  }
  return rows.first().toMap().value(idKey).toString();
}

QVariantList orderWithInsertedId(const QVariantList &currentOrder,
                                 const QVariantList &availableIds,
                                 const QString &insertedId,
                                 int insertAfterIndex) {
  return orderWithInsertedSelectionId(currentOrder, availableIds, insertedId,
                                      insertAfterIndex);
}

QVariantMap orderedSelectionState(const QVariantList &rows,
                                  const QVariantList &preferredOrder,
                                  int currentIndex, const QString &selectedId,
                                  const QString &idKey) {
  return orderedSelectionStateForRows(rows, preferredOrder, currentIndex,
                                      selectedId, idKey);
}

QVariantMap navigateSelectionState(const QVariantList &rows, int currentIndex,
                                   const QString &selectedId, int delta,
                                   int defaultIndex, const QString &idKey) {
  return navigateSelectionDeltaState(rows, currentIndex, selectedId, delta,
                                     defaultIndex, idKey);
}

QVariantMap deleteReselectionState(const QVariantList &rows,
                                   const QVariantList &preferredOrder,
                                   int currentIndex, const QString &removedId,
                                   const QString &idKey) {
  return deleteReselectionStateForRows(rows, preferredOrder, currentIndex,
                                       removedId, idKey);
}

QString deleteNextSelectionId(const QVariantList &rows,
                              const QString &removedId, int defaultIndex,
                              const QString &idKey) {
  return deleteNextSelectionIdForRows(rows, removedId, defaultIndex, idKey);
}

} // namespace ui
