#include "PackageManager.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <curl/curl.h>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void PackageManager::initialize() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

std::string PackageManager::read_github_config() {
    std::ifstream file("/data/data/com.termux/files/home/Izanami/config/github_remote.json");
    if (!file.is_open()) return "";
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

std::string PackageManager::download_file(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) return "Error: cURL init failed.";
    std::string read_buffer;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    if (res != CURLE_OK) return "Error: cURL failed: " + std::string(curl_easy_strerror(res));
    return read_buffer;
}

std::string PackageManager::fetch_remote_packages() {
    std::string config = read_github_config();
    if (config.empty()) return "Error: No GitHub config found.";
    size_t user_pos = config.find("\"username\": \"") + 13;
    size_t user_end = config.find("\"", user_pos);
    std::string username = config.substr(user_pos, user_end - user_pos);
    size_t repo_pos = config.find("\"repo\": \"") + 9;
    size_t repo_end = config.find("\"", repo_pos);
    std::string repo = config.substr(repo_pos, repo_end - repo_pos);
    std::string registry_url = "https://raw.githubusercontent.com/" + username + "/" + repo + "/main/registry.json";
    std::string registry_data = download_file(registry_url);
    if (registry_data.find("Error:") != std::string::npos) return "Failed to reach GitHub. Check config.";
    return "GitHub packages fetched successfully.";
}
