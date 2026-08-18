#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <cstdint>

enum class ChatRole : uint8_t { USER, ASSISTANT, SYSTEM };
enum class ChatState : uint8_t { IDLE, INFERRING, ERROR_STATE };

struct ChatMessage {
    ChatRole role;
    std::string content;
};

class ChatEngine {
private:
    static ChatEngine* instance;
    std::mutex chat_mutex;
    std::atomic<ChatState> state{ChatState::IDLE};
    std::vector<ChatMessage> history;
    std::string input_buffer;
    size_t cursor_pos_{0}; // Tracks where the blinking cursor is
    std::string streaming_response;
    std::string status_message{"Ready"};
    
    ChatEngine() = default;
    std::string build_prompt() const;
    void inference_thread_func(const std::string& prompt);
    static std::string escape_json(const std::string& s);

public:
    static ChatEngine& get_instance();
    ChatEngine(const ChatEngine&) = delete;
    ChatEngine& operator=(const ChatEngine&) = delete;

    void append_char(unsigned int codepoint);
    void backspace();
    void clear_input();
    void submit();
    void move_cursor_left();
    void move_cursor_right();
    size_t get_cursor_pos() const { return cursor_pos_; }

    ChatState get_state() const { return state.load(std::memory_order_acquire); }
    std::string get_status() const { return status_message; }
    std::vector<ChatMessage> get_history();
    void add_message(ChatRole role, const std::string& content);
    
    std::string get_input() const { return input_buffer; }
    std::string get_streaming_response();
    void stop_inference();
    std::string preprocess_input(const std::string& input);
};
