#include "InferenceEngine.hpp"
#include <sys/stat.h>
#include <dirent.h>
#include <iostream>
#include <algorithm>
#include <cctype>

const std::string MODEL_DIR = "/sdcard/Download/Ai offline models/";

InferenceEngine::~InferenceEngine() {
    std::lock_guard<std::mutex> lock(model_mutex_);
    if (llama_context_) llama_free(llama_context_);
    if (llama_model_) llama_model_free(llama_model_);
}

std::vector<ModelInfo> InferenceEngine::scan_available_models() {
    std::vector<ModelInfo> models;
    DIR* dir = opendir(MODEL_DIR.c_str());
    if (!dir) {
        current_status_ = "Permission Denied. Grant Storage in Android Settings.";
        return models;
    }
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        std::string lower_name = name;
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
        if (lower_name.find(".gguf") != std::string::npos) {
            models.push_back({name, MODEL_DIR + name});
        }
    }
    closedir(dir);
    return models;
}

void InferenceEngine::start_initialization() {
    if (is_loading_ || is_initialized_) return;
    is_loading_ = true;
    current_status_ = "Scanning Models...";
    std::thread(&InferenceEngine::initialize_inference_thread, this).detach();
}

void InferenceEngine::initialize_inference_thread() {
    struct stat dir_stat;
    if (stat(MODEL_DIR.c_str(), &dir_stat) != 0 || !S_ISDIR(dir_stat.st_mode)) {
        current_status_ = "Error: 'Ai offline models' folder missing.";
        is_loading_ = false;
        return;
    }
    auto available_models = scan_available_models();
    if (available_models.empty()) {
        if (current_status_.find("Permission Denied") == std::string::npos) {
            current_status_ = "Error: No .gguf models found in folder.";
        }
        is_loading_ = false;
        return;
    }
    std::string target_model_path = "";
    active_model_name_ = "";
    for (const auto& model : available_models) {
        std::string lower_name = model.name;
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
        if (lower_name.find("qwen") != std::string::npos) {
            target_model_path = model.path;
            active_model_name_ = model.name;
            break;
        }
    }
    if (target_model_path.empty()) {
        target_model_path = available_models[0].path;
        active_model_name_ = available_models[0].name;
    }
    current_status_ = "Loading: " + active_model_name_.substr(0, 15) + "...";
    load_model_internal(target_model_path);
    
    if (llama_model_ && llama_context_) {
        current_status_ = "AI Engine Online.";
        is_initialized_ = true;
    } else {
        current_status_ = "Error: Model load failed.";
    }
    is_loading_ = false;
}

void InferenceEngine::load_model_internal(const std::string& model_path) {
    std::lock_guard<std::mutex> lock(model_mutex_);
    if (llama_context_) { llama_free(llama_context_); llama_context_ = nullptr; }
    if (llama_model_) { llama_model_free(llama_model_); llama_model_ = nullptr; }
    
    llama_backend_init();
    llama_model_params model_params = llama_model_default_params();
    model_params.n_gpu_layers = 0;
    llama_model_ = llama_model_load_from_file(model_path.c_str(), model_params);
    if (!llama_model_) return;
    
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = 2048;
    // Bumped to 8 threads to utilize all Snapdragon 888 cores
    ctx_params.n_threads = 8;
    ctx_params.n_threads_batch = 8;
    llama_context_ = llama_init_from_model(llama_model_, ctx_params);
}

bool InferenceEngine::is_model_loaded() const {
    return is_initialized_.load();
}

void InferenceEngine::swap_model(const ModelInfo& new_model) {
    if (is_loading_ || is_generating_) return;
    is_loading_ = true;
    is_initialized_ = false;
    current_status_ = "Swapping to: " + new_model.name;
    
    std::thread([this, new_model]() {
        load_model_internal(new_model.path);
        if (llama_model_ && llama_context_) {
            active_model_name_ = new_model.name;
            current_status_ = "AI Engine Online.";
            is_initialized_ = true;
        } else {
            current_status_ = "Error: Model swap failed.";
        }
        is_loading_ = false;
    }).detach();
}

void InferenceEngine::stop_generation() {
    stop_flag_.store(true);
}

void InferenceEngine::generate(const std::string& prompt, const std::function<bool(const std::string&)>& callback) {
    if (!is_initialized_ || is_generating_) return;
    generate_internal(prompt, callback);
}

void InferenceEngine::generate_internal(const std::string& prompt, const std::function<bool(const std::string&)>& callback) {
    std::lock_guard<std::mutex> lock(model_mutex_);
    if (!llama_model_ || !llama_context_) return;
    
    is_generating_ = true;
    stop_flag_ = false;
    
    const llama_vocab* vocab = llama_model_get_vocab(llama_model_);
    if (!vocab) {
        current_status_ = "Error: Vocab missing.";
        is_generating_ = false;
        return;
    }
    
    std::vector<llama_token> tokens(llama_n_ctx(llama_context_));
    int n_tokens = llama_tokenize(vocab, prompt.c_str(), prompt.size(), tokens.data(), tokens.size(), true, true);
    if (n_tokens < 0) {
        tokens.resize(-n_tokens);
        n_tokens = llama_tokenize(vocab, prompt.c_str(), prompt.size(), tokens.data(), tokens.size(), true, true);
    }
    tokens.resize(n_tokens);
    
    llama_batch batch = llama_batch_get_one(tokens.data(), tokens.size());
    llama_sampler* sampler = llama_sampler_chain_init(llama_sampler_chain_default_params());
    llama_sampler_chain_add(sampler, llama_sampler_init_greedy());
    
    int decode_result = 0;
    while (true) {
        if (stop_flag_.load()) break;
        
        decode_result = llama_decode(llama_context_, batch);
        if (decode_result != 0) {
            // Diagnostic: Will show on screen if decode fails
            current_status_ = "Decode Error: " + std::to_string(decode_result);
            break;
        }
        
        llama_token new_token_id = llama_sampler_sample(sampler, llama_context_, -1);
        if (new_token_id == llama_token_eos(vocab)) {
            // Diagnostic: Will show on screen if it hits EOS immediately
            if (current_status_ == "Thinking...") current_status_ = "Stopped (EOS)";
            break;
        }
        
        char buf[256];
        int n = llama_token_to_piece(vocab, new_token_id, buf, sizeof(buf), 0, true);
        if (n > 0) {
            std::string token_str(buf, n);
            current_status_ = "Generating...";
            if (!callback(token_str)) break;
        }
        batch = llama_batch_get_one(&new_token_id, 1);
    }
    
    llama_sampler_free(sampler);
    is_generating_ = false;
}

void InferenceEngine::update_ui_status(bool& model_loaded, std::string& status_message, std::string& active_model_name) {
    model_loaded = is_initialized_.load();
    status_message = current_status_;
    active_model_name = active_model_name_;
}
