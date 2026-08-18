#pragma once
#include <string>

class PackageManager {
public:
    static PackageManager& get_instance() {
        static PackageManager instance;
        return instance;
    }
    void initialize();
    std::string fetch_remote_packages();
private:
    PackageManager() = default;
    PackageManager(const PackageManager&) = delete;
    PackageManager& operator=(const PackageManager&) = delete;
    std::string read_github_config();
    std::string download_file(const std::string& url);
};
