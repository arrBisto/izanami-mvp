#include "SoulCore.hpp"
#include "LuaEngine.hpp"
#include <sstream>
#include <fstream>
#include <ctime>

void SoulCore::update(const std::string& user_msg, const std::string& task_class) {
    std::lock_guard<std::mutex> lock(mutex_);
    state_.interaction_count++;
    
    std::string state_json = state_to_json();
    std::string response = call_lua(state_json);
    if (!response.empty()) {
        parse_lua_response(response);
    }
    
    if (task_class == "code") {
        state_.styles.clear();
        state_.styles.push_back("locked_in");
        state_.tags.clear();
        state_.tags.push_back("precision");
        last_fragment_ = "[You are in locked_in mode: precise, technical, no jokes. Stay sharp and helpful.]";
    }
}

std::string SoulCore::get_prompt_fragment() {
    std::lock_guard<std::mutex> lock(mutex_);
    return last_fragment_;
}

void SoulCore::log_episode(const std::string& prompt, const std::string& response) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::ofstream ep("/sdcard/Izanami/memory/episodes_log.txt", std::ios::app);
    if (ep) {
        ep << "---\n" << prompt << "\n===\n" << response << "\n";
    }
}

void SoulCore::decay(float hours_elapsed) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (int i = 0; i < 8; i++) {
        state_.emotions[i] *= (1.0f - 0.1f * hours_elapsed);
        if (state_.emotions[i] < 0.01f) state_.emotions[i] = 0.0f;
    }
}

std::string SoulCore::state_to_json() {
    std::ostringstream js;
    js << "{\"traits\":[";
    for (int i = 0; i < 6; i++) js << state_.traits[i] << (i < 5 ? "," : "");
    js << "],\"mood\":[";
    for (int i = 0; i < 3; i++) js << state_.mood[i] << (i < 2 ? "," : "");
    js << "],\"emotions\":[";
    for (int i = 0; i < 8; i++) js << state_.emotions[i] << (i < 7 ? "," : "");
    js << "],\"interaction_count\":" << state_.interaction_count << "}";
    return js.str();
}

std::string SoulCore::call_lua(const std::string& state_json) {
    return LuaEngine::get_instance().execute_script("/sdcard/Izanami/soul.lua", state_json);
}

void SoulCore::parse_lua_response(const std::string& json) {
    size_t frag_start = json.find("\"fragment\":\"");
    if (frag_start != std::string::npos) {
        frag_start += 12;
        size_t frag_end = json.find("\"", frag_start);
        if (frag_end != std::string::npos) {
            last_fragment_ = json.substr(frag_start, frag_end - frag_start);
        }
    }
}
