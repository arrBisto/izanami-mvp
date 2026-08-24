
#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <sqlite3.h>
struct PendingPatch { int id; std::string name; std::string description; std::string patch_code; };

struct GotchaEntry {
    long id;
    std::string error_signature;
    std::string root_cause;
    std::string fix;
};

struct SnippetEntry {
    long id;
    std::string snippet_id;
    std::string tags;
    std::string description;
    std::string code_payload;
};

class MemoryVault {
public:
    void add_pending_patch(const std::string& name, const std::string& desc, const std::string& code);
    std::vector<PendingPatch> get_pending_patches();
    void delete_pending_patch(int id);
public:
    static MemoryVault& get_instance() {
        static MemoryVault instance;
        return instance;
    }
    bool is_ready() const { return ready_; }
    void LogGotcha(const std::string& error_signature, const std::string& root_cause, const std::string& fix);
    std::vector<GotchaEntry> QueryGotcha(const std::string& keyword);
    void StoreSnippet(const std::string& snippet_id, const std::string& tags, const std::string& description, const std::string& code_payload);
    std::vector<SnippetEntry> FindSnippets(const std::string& tag);
    void SetMemory(const std::string& key, const std::string& value);
    std::string GetMemory(const std::string& key);

private:
    MemoryVault();
    ~MemoryVault();
    MemoryVault(const MemoryVault&) = delete;
    MemoryVault& operator=(const MemoryVault&) = delete;
    void create_schema();

    sqlite3* db_{nullptr};
    mutable std::mutex mutex_;
    bool ready_{false};
};
