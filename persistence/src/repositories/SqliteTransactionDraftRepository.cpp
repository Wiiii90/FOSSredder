/**
 * @file persistence/src/repositories/SqliteTransactionDraftRepository.cpp
 * @brief Implements the SQLite-backed transaction draft repository.
 */

#include "persistence/repositories/SqliteTransactionDraftRepository.h"

#include "core/application/import/draft/TransactionDraft.h"
#include "persistence/SqliteDb.h"
#include "persistence/StmtGuard.h"
#include "persistence/Uuid.h"

#include <sqlite3.h>

#include <stdexcept>
#include <utility>

namespace {

void bindNullableText(persistence::StmtGuard& stmt, int column, const std::string& value)
{
    if (value.empty()) {
        sqlite3_bind_null(stmt.get(), column);
        return;
    }
    stmt.bindText(column, value);
}

void bindNullableBlob(persistence::StmtGuard& stmt, int column, const std::vector<uint8_t>& value)
{
    if (value.empty()) {
        sqlite3_bind_null(stmt.get(), column);
        return;
    }
    sqlite3_bind_blob(stmt.get(),
                      column,
                      value.data(),
                      static_cast<int>(value.size()),
                      SQLITE_TRANSIENT);
}

int nextTransactionDraftPosition(sqlite3* db)
{
    if (!db) {
        return 0;
    }

    persistence::StmtGuard stmt(db, "SELECT COALESCE(MAX(position), -1) + 1 FROM transaction_drafts;");
    if (!stmt) {
        return 0;
    }
    if (stmt.step() != SQLITE_ROW) {
        return 0;
    }

    return stmt.columnInt(0);
}

void removeTransactionDraftProperties(sqlite3* db, const std::string& id)
{
    persistence::StmtGuard stmt(db, "DELETE FROM transaction_draft_properties WHERE transaction_draft_id = ?;");
    if (!stmt) {
        return;
    }

    stmt.bindText(1, id);
    stmt.step();
}

void upsertTransactionDraftProperties(sqlite3* db, const core::application::importing::draft::TransactionDraft& draft)
{
    persistence::StmtGuard stmt(
        db,
        "INSERT OR REPLACE INTO transaction_draft_properties (transaction_draft_id, property_position, property_id) VALUES (?, ?, ?);");
    if (!stmt) {
        return;
    }

    for (std::size_t propertyIndex = 0; propertyIndex < draft.propertyIds.size(); ++propertyIndex) {
        const auto& propertyId = draft.propertyIds[propertyIndex];
        if (propertyId.empty()) {
            continue;
        }

        stmt.reset();
        stmt.bindText(1, draft.id);
        stmt.bindInt(2, static_cast<int>(propertyIndex));
        stmt.bindText(3, propertyId);
        stmt.step();
    }
}

void writeTransactionDraft(sqlite3* db,
                           const std::shared_ptr<core::application::importing::draft::TransactionDraft>& draft,
                           const char* sql,
                           bool autoPosition)
{
    if (!db || !draft) {
        return;
    }

    if (draft->id.empty()) {
        draft->id = persistence::generateUuid();
    }
    if (autoPosition && draft->position < 0) {
        draft->position = nextTransactionDraftPosition(db);
    }

    persistence::StmtGuard stmt(db, sql);
    if (!stmt) {
        return;
    }

    stmt.bindText(1, draft->id);
    bindNullableText(stmt, 2, draft->statementDraftId);
    stmt.bindInt(3, draft->position);
    stmt.bindText(4, draft->name);
    stmt.bindText(5, draft->bookingDate);
    stmt.bindText(6, draft->valuta);
    stmt.bindDouble(7, draft->amount);
    stmt.bindText(8, draft->actorText);
    stmt.bindText(9, draft->propertyText);
    bindNullableText(stmt, 10, draft->actorId);
    stmt.bindInt(11, draft->actorSelected ? 1 : 0);
    bindNullableText(stmt, 12, draft->contractId);
    stmt.bindInt(13, draft->contractSelected ? 1 : 0);
    stmt.bindText(14, draft->metadata);
    bindNullableBlob(stmt, 15, draft->proofImageData);
    stmt.bindText(16, draft->type);
    stmt.bindInt(17, draft->allocatable ? 1 : 0);
    stmt.bindInt(18, draft->allocatableSelected ? 1 : 0);
    stmt.bindInt(19, static_cast<int>(draft->status));
    stmt.bindText(20, draft->createdAt);
    stmt.bindText(21, draft->updatedAt);
    stmt.step();

    removeTransactionDraftProperties(db, draft->id);
    upsertTransactionDraftProperties(db, *draft);
}

std::shared_ptr<core::application::importing::draft::TransactionDraft> readDraftRow(sqlite3* db, persistence::StmtGuard& stmt)
{
    auto draft = std::make_shared<core::application::importing::draft::TransactionDraft>();
    draft->id = stmt.columnText(0);
    draft->statementDraftId = stmt.columnText(1);
    draft->position = stmt.columnInt(2);
    draft->name = stmt.columnText(3);
    draft->bookingDate = stmt.columnText(4);
    draft->valuta = stmt.columnText(5);
    draft->amount = stmt.columnDouble(6);
    draft->actorText = stmt.columnText(7);
    draft->propertyText = stmt.columnText(8);
    draft->actorId = stmt.columnText(9);
    draft->actorSelected = stmt.columnInt(10) != 0;
    draft->contractId = stmt.columnText(11);
    draft->contractSelected = stmt.columnInt(12) != 0;
    draft->metadata = stmt.columnText(13);
    const auto* blob = static_cast<const uint8_t*>(sqlite3_column_blob(stmt.get(), 14));
    const int blobSize = sqlite3_column_bytes(stmt.get(), 14);
    if (blob && blobSize > 0) {
        draft->proofImageData.assign(blob, blob + blobSize);
    }
    draft->type = stmt.columnText(15);
    draft->allocatable = stmt.columnInt(16) != 0;
    draft->allocatableSelected = stmt.columnInt(17) != 0;
    draft->status = static_cast<core::domain::Transaction::Status>(stmt.columnInt(18));
    draft->createdAt = stmt.columnText(19);
    draft->updatedAt = stmt.columnText(20);

    persistence::StmtGuard properties(
        db,
        "SELECT property_id FROM transaction_draft_properties WHERE transaction_draft_id = ? ORDER BY property_position;");
    if (properties) {
        properties.bindText(1, draft->id);
        while (properties.step() == SQLITE_ROW) {
            draft->propertyIds.push_back(properties.columnText(0));
        }
    }

    return draft;
}

} // namespace

struct SqliteTransactionDraftRepository::Impl {
    explicit Impl(std::shared_ptr<SqliteDb> db)
        : db(std::move(db))
    {
    }

    std::shared_ptr<SqliteDb> db;
};

SqliteTransactionDraftRepository::SqliteTransactionDraftRepository(const std::string& dbPath)
    : SqliteTransactionDraftRepository(std::make_shared<SqliteDb>(dbPath))
{
}

SqliteTransactionDraftRepository::SqliteTransactionDraftRepository(std::shared_ptr<SqliteDb> db)
    : pimpl_(std::make_unique<Impl>(std::move(db)))
{
    if (!pimpl_->db) {
        throw std::runtime_error("db is null");
    }
}

SqliteTransactionDraftRepository::~SqliteTransactionDraftRepository() = default;

void SqliteTransactionDraftRepository::addTransactionDraft(const std::shared_ptr<core::application::importing::draft::TransactionDraft>& draft)
{
    const auto db = pimpl_->db->handle();
    writeTransactionDraft(
        db,
        draft,
        "INSERT INTO transaction_drafts (id, statement_draft_id, position, name, booking_date, valuta, amount, actor_text, property_text, actor_id, actor_selected, contract_id, contract_selected, metadata, proof_image_data, type, allocatable, allocatable_selected, status, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);",
        true);
}

std::vector<std::shared_ptr<core::application::importing::draft::TransactionDraft>> SqliteTransactionDraftRepository::getTransactionDrafts() const
{
    std::vector<std::shared_ptr<core::application::importing::draft::TransactionDraft>> drafts;
    const auto db = pimpl_->db->handle();

    persistence::StmtGuard stmt(
        db,
        "SELECT id, statement_draft_id, position, name, booking_date, valuta, amount, actor_text, property_text, actor_id, actor_selected, contract_id, contract_selected, metadata, proof_image_data, type, allocatable, allocatable_selected, status, created_at, updated_at FROM transaction_drafts ORDER BY statement_draft_id, position, id;");
    if (!stmt) {
        return drafts;
    }

    while (stmt.step() == SQLITE_ROW) {
        drafts.push_back(readDraftRow(db, stmt));
    }

    return drafts;
}

std::optional<std::shared_ptr<core::application::importing::draft::TransactionDraft>> SqliteTransactionDraftRepository::getTransactionDraftById(const std::string& id) const
{
    const auto db = pimpl_->db->handle();
    persistence::StmtGuard stmt(
        db,
        "SELECT id, statement_draft_id, position, name, booking_date, valuta, amount, actor_text, property_text, actor_id, actor_selected, contract_id, contract_selected, metadata, proof_image_data, type, allocatable, allocatable_selected, status, created_at, updated_at FROM transaction_drafts WHERE id = ? LIMIT 1;");
    if (!stmt) {
        return std::nullopt;
    }

    stmt.bindText(1, id);
    if (stmt.step() != SQLITE_ROW) {
        return std::nullopt;
    }

    return readDraftRow(db, stmt);
}

void SqliteTransactionDraftRepository::removeTransactionDraft(const std::string& id)
{
    const auto db = pimpl_->db->handle();
    removeTransactionDraftProperties(db, id);

    persistence::StmtGuard stmt(db, "DELETE FROM transaction_drafts WHERE id = ?;");
    if (!stmt) {
        return;
    }
    stmt.bindText(1, id);
    stmt.step();
}

void SqliteTransactionDraftRepository::updateTransactionDraft(const std::shared_ptr<core::application::importing::draft::TransactionDraft>& draft)
{
    const auto db = pimpl_->db->handle();
    writeTransactionDraft(
        db,
        draft,
        "UPDATE transaction_drafts SET statement_draft_id = ?2, position = ?3, name = ?4, booking_date = ?5, valuta = ?6, amount = ?7, actor_text = ?8, property_text = ?9, actor_id = ?10, actor_selected = ?11, contract_id = ?12, contract_selected = ?13, metadata = ?14, proof_image_data = ?15, type = ?16, allocatable = ?17, allocatable_selected = ?18, status = ?19, created_at = ?20, updated_at = ?21 WHERE id = ?1;",
        false);
}

void SqliteTransactionDraftRepository::upsertTransactionDraft(const std::shared_ptr<core::application::importing::draft::TransactionDraft>& draft)
{
    const auto db = pimpl_->db->handle();
    writeTransactionDraft(
        db,
        draft,
        "INSERT OR REPLACE INTO transaction_drafts (id, statement_draft_id, position, name, booking_date, valuta, amount, actor_text, property_text, actor_id, actor_selected, contract_id, contract_selected, metadata, proof_image_data, type, allocatable, allocatable_selected, status, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);",
        true);
}

void SqliteTransactionDraftRepository::clearTransactionDrafts()
{
    const auto db = pimpl_->db->handle();

    persistence::StmtGuard deleteProperties(db, "DELETE FROM transaction_draft_properties;");
    if (deleteProperties) {
        deleteProperties.step();
    }

    persistence::StmtGuard deleteDrafts(db, "DELETE FROM transaction_drafts;");
    if (deleteDrafts) {
        deleteDrafts.step();
    }
}
