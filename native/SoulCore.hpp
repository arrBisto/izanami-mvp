#pragma once
#include <string>
#include <vector>
#include <mutex>

class SoulCore {
public:
    static SoulCore& get_instance() { static SoulCore instance; return instance; }

    struct Tone {
        float arousal = 0.0f, valence = 0.0f, warmth = 0.0f, confidence = 0.0f;
        float flirt_signal = 0.0f, repair_signal = 0.0f, self_focus = 0.5f;
    };

    void update(const std::string& user_msg, const std::string& task_class);
    std::string get_prompt_fragment();
    void log_episode(const std::string& prompt, const std::string& response);
    void decay(float hours_elapsed);

private:
    SoulCore() = default;

    struct State {
        float traits[6] = {0.7f, 0.8f, 0.6f, 0.7f, 1.0f, 0.3f};
        float mood[3] = {0.0f, 0.0f, 0.0f};
        float emotions[8] = {0.3f, 0.4f, 0.3f, 0.2f, 0.1f, 0.1f, 0.1f, 0.1f};
        std::vector<std::string> styles = {"playful", "warm"};
        std::vector<std::string> tags;
        int interaction_count = 0;
    } state_;

    std::string last_fragment_, last_recall_;
    Tone last_tone_;
    float base_caps_ = 0.05f, base_exclam_ = 0.1f;
    std::mutex mutex_;

    std::string call_lua(const std::string& state_json);
    std::string state_to_json(const std::string& recall);
    void parse_lua_response(const std::string& json);
    float salience_of(const std::string& text) const;
    std::string recall_query(const std::string& msg);
    Tone tone_of(const std::string& msg);
    void ensure_table();
    static std::string escape_json(const std::string& s);
};
