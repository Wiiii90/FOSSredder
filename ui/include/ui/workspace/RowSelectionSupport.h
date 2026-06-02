/**
 * @file ui/include/ui/workspace/RowSelectionSupport.h
 * @brief Declares shared QML row selection and ordering helpers.
 */

#pragma once

#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

namespace ui {

int indexOfId(const QVariantList &rows, const QString &id);
int indexOfKeyValue(const QVariantList &rows, const QString &key,
                    const QVariant &value);
int wrappedIndex(int index, int count);
QString navigatedSelectionId(const QVariantList &rows, const QString &currentId,
                             int delta, int defaultIndex = 0,
                             const QString &idKey = QStringLiteral("id"));

QVariantList
displayRowsWithEmpty(const QVariantList &rows, const QString &emptyDisplay,
                     const QString &displayKey = QStringLiteral("display"));
QVariantList rowIds(const QVariantList &rows,
                    const QString &idKey = QStringLiteral("id"));
bool rowHasId(const QVariantList &rows, const QString &id,
              const QString &idKey = QStringLiteral("id"));
QVariantMap rowById(const QVariantList &rows, const QString &id,
                    const QString &idKey = QStringLiteral("id"));
QString rememberedOrFirstRowId(const QVariantList &rows,
                               const QVariantMap &rememberedIds,
                               const QString &ownerId,
                               const QString &idKey = QStringLiteral("id"));

QVariantList orderWithInsertedId(const QVariantList &currentOrder,
                                 const QVariantList &availableIds,
                                 const QString &insertedId,
                                 int insertAfterIndex);
QVariantMap orderedSelectionState(const QVariantList &rows,
                                  const QVariantList &preferredOrder,
                                  int currentIndex, const QString &selectedId,
                                  const QString &idKey = QStringLiteral("id"));
QVariantMap navigateSelectionState(const QVariantList &rows, int currentIndex,
                                   const QString &selectedId, int delta,
                                   int defaultIndex = 0,
                                   const QString &idKey = QStringLiteral("id"));
QVariantMap deleteReselectionState(const QVariantList &rows,
                                   const QVariantList &preferredOrder,
                                   int currentIndex, const QString &removedId,
                                   const QString &idKey = QStringLiteral("id"));
QString deleteNextSelectionId(const QVariantList &rows,
                              const QString &removedId, int defaultIndex = 0,
                              const QString &idKey = QStringLiteral("id"));

} // namespace ui
