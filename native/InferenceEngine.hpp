#pragma once
#include <string>
#include <atomic>
#include <thread>
#include <vector>
#include <functional>
#include <mutex>
#include "llama.h"

struct ModelInfo { 
    std::string name; 
    std::string path; 
};

class InferenceEngine {
public:
    static InferenceEngine& get_instance() {
        static InferenceEngine instance;
        return instance;
    }
    void start_initialization();
    void update_ui_status(bool& model_loaded, std::string& status_message, std::string& active_model_name);
    std::vector<ModelInfo> scan_available_models();
    
    // Phase 4 Additions
    bool is_model_loaded() const;
    void swap_model(const ModelInfo& new_model);
    void hot_swap_to_path(const std::string& path);
    std::string get_active_model_name() { return active_model_name_; }
    void set_status(const std::string& s) { current_status_ = s; }
    void stop_generation();
    void generate(const std::string& prompt, const std::function<bool(const std::string&)>& callback);

private:
    InferenceEngine() = default;
    ~InferenceEngine();
    InferenceEngine(const InferenceEngine&) = delete;
    InferenceEngine& operator=(const InferenceEngine&) = delete;
    void initialize_inference_thread();
    void load_model_internal(const std::string& model_path);
    void generate_internal(const std::string& prompt, const std::function<bool(const std::string&)>& callback);

    std::atomic<bool> is_initialized_{false};
    std::atomic<bool> is_loading_{false};
    std::atomic<bool> is_generating_{false};
    std::atomic<bool> stop_flag_{false};
    std::string current_status_{"Idle"};
    std::string active_model_name_{"None"};
    llama_model* llama_model_{nullptr};
    llama_context* llama_context_{nullptr};
    std::mutex model_mutex_;
};
