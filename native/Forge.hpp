#pragma once
#include <string>

class Forge {
public:
    static Forge& get_instance() {
        static Forge instance;
        return instance;
    }
    bool RequestBuild(const std::string& project_dir, const std::string& package_name, const std::string& app_label);
    std::string PollResult();
    bool IsBuildPending() const { return pending_; }

private:
    Forge() = default;
    bool pending_{false};
};
