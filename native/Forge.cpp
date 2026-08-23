#include "Forge.hpp"
#include <fstream>
#include <cstdio>

static const char* QUEUE_DIR = "/sdcard/Izanami/queue/";

bool Forge::RequestBuild(const std::string& project_dir, const std::string& package_name, const std::string& app_label) {
    std::remove((std::string(QUEUE_DIR) + "build_result.txt").c_str());
    if (pending_) return false;
    std::ofstream req(std::string(QUEUE_DIR) + "build_request.txt");
    if (!req.is_open()) return false;
    req << "project_dir=" << project_dir << "\n";
    req << "package_name=" << package_name << "\n";
    req << "app_label=" << app_label << "\n";
    req.close();
    pending_ = true;
    return true;
}

std::string Forge::PollResult() {
    if (!pending_) return "";
    std::ifstream res(std::string(QUEUE_DIR) + "build_result.txt");
    if (!res.is_open()) return "";
    std::string line;
    std::string status = "UNKNOWN";
    std::string apk = "";
    while (std::getline(res, line)) {
        if (line.rfind("status=", 0) == 0) status = line.substr(7);
        if (line.rfind("apk_path=", 0) == 0) apk = line.substr(9);
    }
    res.close();
    std::remove((std::string(QUEUE_DIR) + "build_result.txt").c_str());
    pending_ = false;
    std::string msg = "FORGE: build " + status + ".";
    if (!apk.empty()) msg += " APK ready: " + apk;
    return msg;
}
