#include "ChatEngine.hpp"
#include "InferenceEngine.hpp"
#include "LuaEngine.hpp"
#include "Forge.hpp"
#include <thread>
#include <sstream>
#include <fstream>
#include <sys/stat.h>

ChatEngine* ChatEngine::instance = nullptr;

ChatEngine& ChatEngine::get_instance() {
    if (!instance) {
        instance = new ChatEngine();
    }
    return *instance;
}

void ChatEngine::append_char(unsigned int codepoint) {
    if (state.load(std::memory_order_acquire) == ChatState::INFERRING) return;
    char c = static_cast<char>(codepoint);
    input_buffer.insert(cursor_pos_, 1, c);
    cursor_pos_++;
}

void ChatEngine::backspace() {
    if (state.load(std::memory_order_acquire) == ChatState::INFERRING) return;
    if (cursor_pos_ == 0 || input_buffer.empty()) return;
    
    size_t pos = cursor_pos_ - 1;
    while (pos > 0 && (input_buffer[pos] & 0xC0) == 0x80) {
        --pos;
    }
    input_buffer.erase(pos, cursor_pos_ - pos);
    cursor_pos_ = pos;
}

void ChatEngine::move_cursor_left() {
    if (cursor_pos_ > 0) {
        cursor_pos_--;
        while (cursor_pos_ > 0 && (input_buffer[cursor_pos_] & 0xC0) == 0x80) {
            cursor_pos_--;
        }
    }
}

void ChatEngine::move_cursor_right() {
    if (cursor_pos_ < input_buffer.size()) {
        cursor_pos_++;
        while (cursor_pos_ < input_buffer.size() && (input_buffer[cursor_pos_] & 0xC0) == 0x80) {
            cursor_pos_++;
        }
    }
}

void ChatEngine::clear_input() { 
    input_buffer.clear(); 
    cursor_pos_ = 0;
}

std::string ChatEngine::escape_json(const std::string& s) {
    std::string result;
    result.reserve(s.size() + 8);
    for (char c : s) {
        switch (c) {
            case '"':  result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n";  break;
            case '\r': result += "\\r";  break;
            case '\t': result += "\\t";  break;
            default:   result += c;      break;
        }
    }
    return result;
}

std::string ChatEngine::build_prompt() const {
    std::ostringstream prompt;
    prompt << "<|im_start|>system\nYou are Izanami, a helpful AI assistant running natively on Android. Be concise and helpful.<|im_end|>\n";
    for (const auto& msg : history) {
        const char* role_str = "user";
        switch (msg.role) {
            case ChatRole::USER:      role_str = "user";      break;
            case ChatRole::ASSISTANT: role_str = "assistant";  break;
            case ChatRole::SYSTEM:    role_str = "system";    break;
        }
        prompt << "<|im_start|>" << role_str << "\n" << msg.content << "<|im_end|>\n";
    }
    prompt << "<|im_start|>assistant\n";
    return prompt.str();
}

std::string ChatEngine::preprocess_input(const std::string& input) {
    struct stat st;
    if (stat("/sdcard/Izanami/packages/preprocess.lua", &st) == 0) {
        std::string json_param = "{\"input\":\"" + escape_json(input) + "\"}";
        std::string lua_result = LuaEngine::get_instance().execute_script("/sdcard/Izanami/packages/preprocess.lua", json_param);
        if (!lua_result.empty()) return lua_result;
    }
    return input;
}

void ChatEngine::inference_thread_func(const std::string& prompt) {
    auto& engine = InferenceEngine::get_instance();
    {
        std::lock_guard<std::mutex> lock(chat_mutex);
        streaming_response.clear();
    }
    engine.generate(prompt, [this](const std::string& token) -> bool {
        std::lock_guard<std::mutex> lock(chat_mutex);
        streaming_response += token;
        status_message = "Generating...";
        return true;
    });
    
    {
        std::lock_guard<std::mutex> lock(chat_mutex);
        if (!streaming_response.empty()) {
            history.push_back({ChatRole::ASSISTANT, streaming_response});
            status_message = "Ready";
        } else {
            bool dummy_model_loaded;
            std::string engine_status;
            std::string dummy_model_name;
            engine.update_ui_status(dummy_model_loaded, engine_status, dummy_model_name);
            if (engine_status.find("Decode Error") != std::string::npos) status_message = engine_status;
            else if (engine_status.find("Stopped") != std::string::npos) status_message = engine_status;
            else status_message = "Finished (No Output)";
        }
        streaming_response.clear();
    }
    state.store(ChatState::IDLE, std::memory_order_release);
}

void ChatEngine::submit() {
    if (state.load(std::memory_order_acquire) == ChatState::INFERRING) return;
    if (input_buffer.empty()) return;
    if (input_buffer.rfind("/save ", 0) == 0) {
        std::string fname = input_buffer.substr(6);
        std::lock_guard<std::mutex> lock(chat_mutex);
        history.push_back({ChatRole::USER, input_buffer});
        std::string save_prompt = "Save the following as a clean text file named " + fname + ":\n\n";
        if (!history.empty() && history.back().role == ChatRole::ASSISTANT) {
            save_prompt += history.back().content;
        } else {
            save_prompt += "No content to save.";
        }
        history.push_back({ChatRole::ASSISTANT, "Preparing to save " + fname + ".txt..."});
        input_buffer.clear();
        save_to_file(fname, save_prompt);
        return;
    }
    if (input_buffer.rfind("/forge ", 0) == 0) {
        std::istringstream ss(input_buffer.substr(7));
        std::string proj;
        std::string pkg;
        std::string label;
        ss >> proj >> pkg >> label;
        if (!proj.empty() && proj.compare(0, 1, "/") != 0) proj = "/data/data/com.termux/files/home/Izanami/workspace/" + proj;
        if (Forge::get_instance().RequestBuild(proj, pkg, label)) {
            std::lock_guard<std::mutex> lock(chat_mutex);
            history.push_back({ChatRole::USER, input_buffer});
            status_message = "Forge request sent. Waiting for Bridge...";
        } else {
            status_message = "Forge busy or queue error.";
        }
        input_buffer.clear();
        return;
    }
    if (!InferenceEngine::get_instance().is_model_loaded()) {
        status_message = "Model not loaded";
        return;
    }
    std::string processed = preprocess_input(input_buffer);
    {
        std::lock_guard<std::mutex> lock(chat_mutex);
        history.push_back({ChatRole::USER, input_buffer});
    }
    clear_input();
    std::string prompt = build_prompt();
    state.store(ChatState::INFERRING, std::memory_order_release);
    status_message = "Thinking...";
    std::thread([this, prompt]() {
        inference_thread_func(prompt);
    }).detach();
}

std::vector<ChatMessage> ChatEngine::get_history() {
    std::lock_guard<std::mutex> lock(chat_mutex);
    return history;
}

std::string ChatEngine::get_streaming_response() {
    std::lock_guard<std::mutex> lock(chat_mutex);
    return streaming_response;
}

void ChatEngine::stop_inference() {
    InferenceEngine::get_instance().stop_generation();
    status_message = "Stopping...";
}

void ChatEngine::add_message(ChatRole role, const std::string& content) {
    std::lock_guard<std::mutex> lock(chat_mutex);
    history.push_back({role, content});
}

void ChatEngine::append_string(const std::string& utf8_text) {
    size_t i = 0;
    while (i < utf8_text.size()) {
        unsigned char c = (unsigned char)utf8_text[i];
        unsigned int cp = 0;
        size_t len = 1;
        if (c < 0x80) { cp = c; }
        else if ((c >> 5) == 0x6) { cp = c & 0x1F; len = 2; }
        else if ((c >> 4) == 0xE) { cp = c & 0x0F; len = 3; }
        else if ((c >> 3) == 0x1E) { cp = c & 0x07; len = 4; }
        for (size_t k = 1; k < len && i + k < utf8_text.size(); ++k) {
            cp = (cp << 6) | ((unsigned char)utf8_text[i + k] & 0x3F);
        }
        i += len;
        append_char(cp);
    }
}

void ChatEngine::set_input(const std::string& text) {
    std::lock_guard<std::mutex> lock(chat_mutex);
    input_buffer = text;
    cursor_pos_ = text.size();
}

void ChatEngine::save_to_file(const std::string& filename, const std::string& content) {
    std::string path = "/sdcard/Download/Izanami/";
    mkdir(path.c_str(), 0755);
    std::string full_path = path + filename + ".txt";
    std::ofstream file(full_path);
    if (file.is_open()) {
        file << content;
        file.close();
        std::lock_guard<std::mutex> lock(chat_mutex);
        status_message = "Saved: " + filename + ".txt";
    } else {
        std::lock_guard<std::mutex> lock(chat_mutex);
        status_message = "Save failed";
    }
}
