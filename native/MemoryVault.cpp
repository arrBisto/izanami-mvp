#include "MemoryVault.hpp"
#include <ctime>

static const char* VAULT_DB_PATH = "/sdcard/Izanami/memory/vault.db";

static std::string current_timestamp() {
    char buf[32];
    time_t now = time(nullptr);
    struct tm tm_info;
    localtime_r(&now, &tm_info);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm_info);
    return std::string(buf);
}

MemoryVault::MemoryVault() {
    if (sqlite3_open(VAULT_DB_PATH, &db_) != SQLITE_OK) {
        if (db_) sqlite3_close(db_);
        db_ = nullptr;
        return;
    }
    create_schema();
    ready_ = true;
    SetMemory("last_boot", current_timestamp());
}

MemoryVault::~MemoryVault() {
    if (db_) sqlite3_close(db_);
}

void MemoryVault::create_schema() {
    const char* schema =
        "CREATE TABLE IF NOT EXISTS gotcha_ledger ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "error_signature TEXT NOT NULL, "
        "root_cause TEXT, "
        "fix TEXT, "
        "created_at TEXT); "
        "CREATE TABLE IF NOT EXISTS code_vault ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "snippet_id TEXT NOT NULL UNIQUE, "
        "tags TEXT, "
        "description TEXT, "
        "code_payload TEXT, "
        "checksum TEXT, "
        "created_at TEXT); "
        "CREATE TABLE IF NOT EXISTS memory_core ("
        "key TEXT PRIMARY KEY, "
        "value TEXT, "
        "updated_at TEXT); "
        "CREATE TABLE IF NOT EXISTS doc_index ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "doc_name TEXT, "
        "heading_path TEXT, "
        "tags TEXT, "
        "content TEXT);";
    sqlite3_exec(db_, schema, nullptr, nullptr, nullptr);
}

void MemoryVault::LogGotcha(const std::string& error_signature, const std::string& root_cause, const std::string& fix) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!ready_) return;
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "INSERT INTO gotcha_ledger (error_signature, root_cause, fix, created_at) VALUES (?1, ?2, ?3, ?4);";
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, error_signature.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, root_cause.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, fix.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, current_timestamp().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::vector<GotchaEntry> MemoryVault::QueryGotcha(const std::string& keyword) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<GotchaEntry> results;
    if (!ready_) return results;
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, error_signature, root_cause, fix FROM gotcha_ledger "
                      "WHERE error_signature LIKE ?1 OR root_cause LIKE ?1 OR fix LIKE ?1 LIMIT 5;";
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return results;
    std::string pattern = "%" + keyword + "%";
    sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        GotchaEntry entry;
        entry.id = sqlite3_column_int64(stmt, 0);
        const unsigned char* sig = sqlite3_column_text(stmt, 1);
        const unsigned char* cause = sqlite3_column_text(stmt, 2);
        const unsigned char* fix = sqlite3_column_text(stmt, 3);
        entry.error_signature = sig ? reinterpret_cast<const char*>(sig) : "";
        entry.root_cause = cause ? reinterpret_cast<const char*>(cause) : "";
        entry.fix = fix ? reinterpret_cast<const char*>(fix) : "";
        results.push_back(entry);
    }
    sqlite3_finalize(stmt);
    return results;
}

void MemoryVault::StoreSnippet(const std::string& snippet_id, const std::string& tags, const std::string& description, const std::string& code_payload) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!ready_) return;
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "INSERT OR REPLACE INTO code_vault (snippet_id, tags, description, code_payload, created_at) VALUES (?1, ?2, ?3, ?4, ?5);";
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, snippet_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, tags.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, code_payload.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, current_timestamp().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::vector<SnippetEntry> MemoryVault::FindSnippets(const std::string& tag) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<SnippetEntry> results;
    if (!ready_) return results;
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT id, snippet_id, tags, description, code_payload FROM code_vault "
                      "WHERE tags LIKE ?1 LIMIT 5;";
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return results;
    std::string pattern = "%" + tag + "%";
    sqlite3_bind_text(stmt, 1, pattern.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        SnippetEntry entry;
        entry.id = sqlite3_column_int64(stmt, 0);
        const unsigned char* sid = sqlite3_column_text(stmt, 1);
        const unsigned char* tags = sqlite3_column_text(stmt, 2);
        const unsigned char* desc = sqlite3_column_text(stmt, 3);
        const unsigned char* payload = sqlite3_column_text(stmt, 4);
        entry.snippet_id = sid ? reinterpret_cast<const char*>(sid) : "";
        entry.tags = tags ? reinterpret_cast<const char*>(tags) : "";
        entry.description = desc ? reinterpret_cast<const char*>(desc) : "";
        entry.code_payload = payload ? reinterpret_cast<const char*>(payload) : "";
        results.push_back(entry);
    }
    sqlite3_finalize(stmt);
    return results;
}

void MemoryVault::SetMemory(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!ready_) return;
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "INSERT OR REPLACE INTO memory_core (key, value, updated_at) VALUES (?1, ?2, ?3);";
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return;
    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, value.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, current_timestamp().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::string MemoryVault::GetMemory(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!ready_) return "";
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT value FROM memory_core WHERE key = ?1;";
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) return "";
    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    std::string result = "";
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* val = sqlite3_column_text(stmt, 0);
        if (val) result = reinterpret_cast<const char*>(val);
    }
    sqlite3_finalize(stmt);
    return result;
}
