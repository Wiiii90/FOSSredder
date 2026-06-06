/**
 * @file persistence/src/SqliteSchema.cpp
 * @brief Implements SQLite schema initialization and versioning.
 */

#include "persistence/SqliteSchema.h"

#include <sqlite3.h>

#include <stdexcept>
#include <string>

static void exec(sqlite3* db, const char* sql)
{
    char* err = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err) != SQLITE_OK) {
        std::string message = err ? err : "unknown";
        if (err) {
            sqlite3_free(err);
        }
        throw std::runtime_error(message);
    }
}

int SqliteSchema::getUserVersion(sqlite3* db)
{
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "PRAGMA user_version;", -1, &stmt, nullptr) != SQLITE_OK) {
        return 0;
    }

    int version = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        version = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return version;
}

void SqliteSchema::setUserVersion(sqlite3* db, int v)
{
    exec(db, (std::string("PRAGMA user_version = ") + std::to_string(v) + ";").c_str());
}

void SqliteSchema::ensure(sqlite3* db)
{
    if (!db) {
        throw std::runtime_error("db is null");
    }

    exec(db, "PRAGMA foreign_keys = ON;");
    migrate(db);
}

void SqliteSchema::migrate(sqlite3* db)
{
    const int version = getUserVersion(db);
    exec(db,
         "BEGIN;"
         "CREATE TABLE IF NOT EXISTS configs (name TEXT PRIMARY KEY, value TEXT);"
         "CREATE TABLE IF NOT EXISTS actors ("
         "id TEXT PRIMARY KEY,"
         "name TEXT NOT NULL UNIQUE,"
         "created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
         "updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP"
         ");"
         "CREATE TABLE IF NOT EXISTS actor_aliases ("
         "actor_id TEXT NOT NULL,"
         "alias TEXT NOT NULL,"
         "hit_count INTEGER NOT NULL DEFAULT 1,"
         "created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
         "updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
         "last_used_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
         "PRIMARY KEY(actor_id, alias),"
         "FOREIGN KEY(actor_id) REFERENCES actors(id) ON DELETE CASCADE"
         ");"
         "CREATE TABLE IF NOT EXISTS properties ("
         "id TEXT PRIMARY KEY,"
         "name TEXT NOT NULL UNIQUE,"
         "created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
         "updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP"
         ");"
         "CREATE TABLE IF NOT EXISTS property_aliases ("
         "property_id TEXT NOT NULL,"
         "alias TEXT NOT NULL,"
         "hit_count INTEGER NOT NULL DEFAULT 1,"
         "created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
         "updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
         "last_used_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
         "PRIMARY KEY(property_id, alias),"
         "FOREIGN KEY(property_id) REFERENCES properties(id) ON DELETE CASCADE"
         ");"
         "CREATE TABLE IF NOT EXISTS contracts ("
         "id TEXT PRIMARY KEY,"
         "name TEXT NOT NULL,"
         "type TEXT NOT NULL DEFAULT '',"
         "allocatable_mode TEXT NOT NULL DEFAULT 'mixed',"
         "created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
         "updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP"
         ");"
         "CREATE TABLE IF NOT EXISTS contract_aliases ("
         "contract_id TEXT NOT NULL,"
         "alias TEXT NOT NULL,"
         "hit_count INTEGER NOT NULL DEFAULT 1,"
         "created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
         "updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
         "last_used_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
         "PRIMARY KEY(contract_id, alias),"
         "FOREIGN KEY(contract_id) REFERENCES contracts(id) ON DELETE CASCADE"
         ");"
         "CREATE TABLE IF NOT EXISTS contract_actors ("
         "contract_id TEXT NOT NULL,"
         "actor_id TEXT NOT NULL,"
         "PRIMARY KEY(contract_id, actor_id),"
         "FOREIGN KEY(contract_id) REFERENCES contracts(id) ON DELETE CASCADE,"
         "FOREIGN KEY(actor_id) REFERENCES actors(id) ON DELETE CASCADE"
         ");"
         "CREATE TABLE IF NOT EXISTS contract_properties ("
         "contract_id TEXT NOT NULL,"
         "property_id TEXT NOT NULL,"
         "PRIMARY KEY(contract_id, property_id),"
         "FOREIGN KEY(contract_id) REFERENCES contracts(id) ON DELETE CASCADE,"
         "FOREIGN KEY(property_id) REFERENCES properties(id) ON DELETE CASCADE"
         ");"
         "CREATE TABLE IF NOT EXISTS statements ("
         "id TEXT PRIMARY KEY,"
         "name TEXT NOT NULL DEFAULT '',"
         "created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
         "updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP"
         ");"
         "CREATE TABLE IF NOT EXISTS statement_transactions ("
         "statement_id TEXT NOT NULL,"
         "transaction_id TEXT NOT NULL,"
         "position INTEGER NOT NULL DEFAULT 0,"
         "PRIMARY KEY(statement_id, transaction_id),"
         "FOREIGN KEY(statement_id) REFERENCES statements(id) ON DELETE CASCADE,"
         "FOREIGN KEY(transaction_id) REFERENCES transactions(id) ON DELETE CASCADE"
         ");"
         "CREATE TABLE IF NOT EXISTS statement_drafts ("
         "id TEXT PRIMARY KEY,"
         "name TEXT NOT NULL DEFAULT '',"
         "created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
         "updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP"
         ");"
         "CREATE TABLE IF NOT EXISTS statement_draft_transactions ("
         "statement_draft_id TEXT NOT NULL,"
         "transaction_draft_id TEXT NOT NULL,"
         "position INTEGER NOT NULL DEFAULT 0,"
         "PRIMARY KEY(statement_draft_id, transaction_draft_id),"
         "FOREIGN KEY(statement_draft_id) REFERENCES statement_drafts(id) ON DELETE CASCADE,"
         "FOREIGN KEY(transaction_draft_id) REFERENCES transaction_drafts(id) ON DELETE CASCADE"
         ");"
         "CREATE TABLE IF NOT EXISTS transaction_drafts ("
         "id TEXT PRIMARY KEY,"
         "statement_draft_id TEXT,"
         "position INTEGER NOT NULL DEFAULT 0,"
         "name TEXT NOT NULL DEFAULT '',"
         "booking_date TEXT NOT NULL DEFAULT '',"
         "valuta TEXT NOT NULL DEFAULT '',"
         "amount REAL NOT NULL DEFAULT 0,"
         "actor_text TEXT NOT NULL DEFAULT '',"
         "property_text TEXT NOT NULL DEFAULT '',"
         "actor_id TEXT NOT NULL DEFAULT '',"
         "actor_selected INTEGER NOT NULL DEFAULT 0,"
         "contract_id TEXT NOT NULL DEFAULT '',"
         "contract_selected INTEGER NOT NULL DEFAULT 0,"
         "metadata TEXT NOT NULL DEFAULT '',"
         "proof_image_data BLOB,"
         "type TEXT NOT NULL DEFAULT '',"
         "allocatable INTEGER NOT NULL DEFAULT 0,"
         "allocatable_selected INTEGER NOT NULL DEFAULT 0,"
         "status INTEGER NOT NULL DEFAULT 1,"
         "created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
         "updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
         "FOREIGN KEY(statement_draft_id) REFERENCES statement_drafts(id) ON DELETE CASCADE"
         ");"
         "CREATE TABLE IF NOT EXISTS transaction_draft_properties ("
         "transaction_draft_id TEXT NOT NULL,"
         "property_position INTEGER NOT NULL,"
         "property_id TEXT NOT NULL,"
         "PRIMARY KEY(transaction_draft_id, property_position),"
         "FOREIGN KEY(transaction_draft_id) REFERENCES transaction_drafts(id) ON DELETE CASCADE"
         ");"
         "CREATE TABLE IF NOT EXISTS transactions ("
         "id TEXT PRIMARY KEY,"
         "name TEXT NOT NULL DEFAULT '',"
         "booking_date TEXT NOT NULL DEFAULT '',"
         "amount REAL NOT NULL DEFAULT 0,"
         "statement_id TEXT,"
         "status INTEGER NOT NULL DEFAULT 0,"
         "actor_id TEXT,"
         "contract_id TEXT,"
         "valuta TEXT NOT NULL DEFAULT '',"
         "allocatable INTEGER NOT NULL DEFAULT 0,"
         "created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
         "updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
         "FOREIGN KEY(actor_id) REFERENCES actors(id) ON DELETE SET NULL,"
         "FOREIGN KEY(contract_id) REFERENCES contracts(id) ON DELETE SET NULL,"
         "FOREIGN KEY(statement_id) REFERENCES statements(id) ON DELETE CASCADE"
         ");"
         "CREATE TABLE IF NOT EXISTS transaction_properties ("
         "transaction_id TEXT NOT NULL,"
         "property_id TEXT NOT NULL,"
         "PRIMARY KEY(transaction_id, property_id),"
         "FOREIGN KEY(transaction_id) REFERENCES transactions(id) ON DELETE CASCADE,"
         "FOREIGN KEY(property_id) REFERENCES properties(id) ON DELETE CASCADE"
         ");"
         "CREATE TABLE IF NOT EXISTS analyses ("
         "id TEXT PRIMARY KEY,"
         "name TEXT NOT NULL DEFAULT '',"
         "type TEXT NOT NULL DEFAULT '',"
         "config_json TEXT NOT NULL DEFAULT '',"
         "filter_spec TEXT NOT NULL DEFAULT '',"
         "export_format TEXT NOT NULL DEFAULT '',"
         "include_calc_adjustments INTEGER NOT NULL DEFAULT 1,"
         "export_state_json TEXT NOT NULL DEFAULT '{}',"
         "snapshot_transactions_json TEXT NOT NULL DEFAULT '{}',"
         "created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
         "updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP"
         ");"
         "CREATE TABLE IF NOT EXISTS analysis_adjustments ("
         "analysis_id TEXT NOT NULL,"
         "adjustment_key TEXT NOT NULL,"
         "adjustment_value REAL NOT NULL DEFAULT 0,"
         "position INTEGER NOT NULL DEFAULT 0,"
         "PRIMARY KEY(analysis_id, adjustment_key),"
         "FOREIGN KEY(analysis_id) REFERENCES analyses(id) ON DELETE CASCADE"
         ");"
         "CREATE TABLE IF NOT EXISTS annuals ("
         "id TEXT PRIMARY KEY,"
         "name TEXT NOT NULL DEFAULT '',"
         "year INTEGER NOT NULL DEFAULT 0,"
         "created_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
         "updated_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP"
         ");"
         "CREATE TABLE IF NOT EXISTS annual_analyses ("
         "annual_id TEXT NOT NULL,"
         "analysis_id TEXT NOT NULL,"
         "position INTEGER NOT NULL DEFAULT 0,"
         "PRIMARY KEY(annual_id, analysis_id),"
         "FOREIGN KEY(annual_id) REFERENCES annuals(id) ON DELETE CASCADE,"
         "FOREIGN KEY(analysis_id) REFERENCES analyses(id) ON DELETE CASCADE"
         ");"
         "CREATE TABLE IF NOT EXISTS import_logs ("
         "id TEXT PRIMARY KEY,"
         "time TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
         "type TEXT NOT NULL DEFAULT '',"
         "file TEXT NOT NULL DEFAULT '',"
         "status TEXT NOT NULL DEFAULT '',"
         "message TEXT NOT NULL DEFAULT '',"
         "draft_attached INTEGER NOT NULL DEFAULT 0,"
         "draft_id TEXT NOT NULL DEFAULT '',"
         "statement_id TEXT NOT NULL DEFAULT ''"
         ");"
         "CREATE TABLE IF NOT EXISTS import_log_statement_drafts ("
         "import_log_id TEXT NOT NULL,"
         "statement_draft_id TEXT NOT NULL,"
         "position INTEGER NOT NULL DEFAULT 0,"
         "PRIMARY KEY(import_log_id, statement_draft_id),"
         "FOREIGN KEY(import_log_id) REFERENCES import_logs(id) ON DELETE CASCADE,"
         "FOREIGN KEY(statement_draft_id) REFERENCES statement_drafts(id) ON DELETE CASCADE"
         ");"
         "CREATE TABLE IF NOT EXISTS export_logs ("
         "id TEXT PRIMARY KEY,"
         "time TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,"
         "target_path TEXT NOT NULL DEFAULT '',"
         "status TEXT NOT NULL DEFAULT '',"
         "message TEXT NOT NULL DEFAULT '',"
         "payload TEXT NOT NULL DEFAULT ''"
         ");"
         "CREATE TABLE IF NOT EXISTS export_log_annuals ("
         "export_log_id TEXT NOT NULL,"
         "annual_id TEXT NOT NULL,"
         "position INTEGER NOT NULL DEFAULT 0,"
         "PRIMARY KEY(export_log_id, annual_id),"
         "FOREIGN KEY(export_log_id) REFERENCES export_logs(id) ON DELETE CASCADE,"
         "FOREIGN KEY(annual_id) REFERENCES annuals(id) ON DELETE CASCADE"
         ");"
         "CREATE TABLE IF NOT EXISTS export_log_analyses ("
         "export_log_id TEXT NOT NULL,"
         "analysis_id TEXT NOT NULL,"
         "position INTEGER NOT NULL DEFAULT 0,"
         "PRIMARY KEY(export_log_id, analysis_id),"
         "FOREIGN KEY(export_log_id) REFERENCES export_logs(id) ON DELETE CASCADE,"
         "FOREIGN KEY(analysis_id) REFERENCES analyses(id) ON DELETE CASCADE"
         ");"
         "CREATE INDEX IF NOT EXISTS idx_transactions_contract_id ON transactions(contract_id);"
         "CREATE INDEX IF NOT EXISTS idx_contract_actors_actor_id ON contract_actors(actor_id);"
         "CREATE INDEX IF NOT EXISTS idx_contract_properties_property_id ON contract_properties(property_id);"
         "CREATE INDEX IF NOT EXISTS idx_transaction_properties_property_id ON transaction_properties(property_id);"
         "CREATE INDEX IF NOT EXISTS idx_statement_transactions_transaction_id ON statement_transactions(transaction_id);"
         "CREATE INDEX IF NOT EXISTS idx_statement_draft_transactions_transaction_draft_id ON statement_draft_transactions(transaction_draft_id);"
         "CREATE INDEX IF NOT EXISTS idx_actor_aliases_alias ON actor_aliases(alias);"
         "CREATE INDEX IF NOT EXISTS idx_property_aliases_alias ON property_aliases(alias);"
         "CREATE INDEX IF NOT EXISTS idx_contract_aliases_alias ON contract_aliases(alias);"
         "CREATE INDEX IF NOT EXISTS idx_import_logs_time ON import_logs(time DESC);"
         "CREATE INDEX IF NOT EXISTS idx_export_logs_time ON export_logs(time DESC);"
         "CREATE INDEX IF NOT EXISTS idx_analysis_adjustments_analysis_id ON analysis_adjustments(analysis_id);"
         "CREATE INDEX IF NOT EXISTS idx_annual_analyses_analysis_id ON annual_analyses(analysis_id);"
         "CREATE INDEX IF NOT EXISTS idx_import_log_statement_drafts_statement_draft_id ON import_log_statement_drafts(statement_draft_id);"
         "CREATE INDEX IF NOT EXISTS idx_export_log_annuals_annual_id ON export_log_annuals(annual_id);"
         "CREATE INDEX IF NOT EXISTS idx_export_log_analyses_analysis_id ON export_log_analyses(analysis_id);"
         "COMMIT;");

    if (version >= 1 && version < 2) {
        exec(db, "ALTER TABLE contracts ADD COLUMN allocatable_mode TEXT NOT NULL DEFAULT 'mixed';");
        setUserVersion(db, 2);
    } else if (version < 1) {
        setUserVersion(db, 2);
    }
}
