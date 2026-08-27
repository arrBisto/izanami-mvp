#include <fstream>
#include "InferenceEngine.hpp"
#include <cstdio>
#include "Attach.hpp"
#include "mtmd.h"
#include "mtmd-helper.h"
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
            if (lower_name.find("mmproj") != std::string::npos) continue;
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
    {
        std::ifstream pf("/sdcard/Izanami/memory/active_model.txt");
        if (false) {
            std::string pref;
            std::getline(pf, pref);
            struct stat pst;
            if (!pref.empty() && stat(pref.c_str(), &pst) == 0) {
                target_model_path = pref;
                active_model_name_ = pref.substr(pref.find_last_of("/") + 1);
            }
        }
    }
    if (target_model_path.empty()) active_model_name_ = "";
    for (const auto& model : available_models) {
        std::string lower_name = model.name;
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
        if (target_model_path.empty() && lower_name.find("qwen") != std::string::npos) {
            target_model_path = model.path;
            active_model_name_ = model.name;
            break;
        }
    }
    for (const auto& model : available_models) {
        std::string lower_name = model.name;
        std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
        if (target_model_path.empty() && lower_name.find("qwen") != std::string::npos) { target_model_path = model.path; active_model_name_ = model.name; break; }
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

void InferenceEngine::generate(const std::string& prompt, const std::string& image_path, const std::function<bool(const std::string&)>& callback) {
    if (!is_initialized_ || is_generating_) return;
    generate_internal(prompt, image_path, callback);
}

void InferenceEngine::generate(const std::string& prompt, const std::function<bool(const std::string&)>& callback) {
    generate(prompt, "", callback);
}

void InferenceEngine::set_vision_paths(const std::string& core, const std::string& mmproj) {
    vision_core_path_ = core;
    vision_mmproj_path_ = mmproj;
}

void InferenceEngine::set_mmproj_path(const std::string& path) {
    if (path == mmproj_path_) return;
    if (mtmd_ctx_) { mtmd_free(mtmd_ctx_); mtmd_ctx_ = nullptr; }
    mmproj_path_ = path;
    if (!path.empty() && llama_model_) {
        mtmd_context_params params = mtmd_context_params_default();
        params.n_threads = 8;
        mtmd_ctx_ = mtmd_init_from_file(path.c_str(), llama_model_, params);
    }
}

void InferenceEngine::generate_internal(const std::string& prompt, const std::string& image_path, const std::function<bool(const std::string&)>& callback) {
    std::unique_lock<std::mutex> lock(model_mutex_);
    if (!llama_model_ || !llama_context_) return;

    is_generating_ = true;
    stop_flag_ = false;
    current_status_ = "Thinking...";

    const llama_vocab* vocab = llama_model_get_vocab(llama_model_);
    if (!vocab) {
        current_status_ = "Error: Vocab missing.";
        is_generating_ = false;
        return;
    }

    std::string img_path = image_path;
    if (img_path.empty() && Attach::has_attachment()) img_path = Attach::attachment_path();
    if (img_path.empty()) { std::ifstream li("/sdcard/Izanami/memory/last_image.txt"); if (li) std::getline(li, img_path); }
    { std::ofstream vl("/sdcard/Izanami/memory/vision_log.txt", std::ios::app); vl << "gen img=" << img_path << " core=" << active_model_name_ << "\n"; }
    std::string ptext = prompt;
    {
        size_t ip = ptext.find("[image: ");
        if (ip != std::string::npos) {
            size_t iq = ptext.find(']', ip);
            if (iq != std::string::npos) {
                if (img_path.empty()) img_path = ptext.substr(ip + 8, iq - ip - 8);
                ptext.erase(ip, iq - ip + 1);
            }
        }
    }
    llama_pos n_past = 0;

    if (vision_core_path_.empty()) {
        vision_core_path_ = "/sdcard/Download/Ai offline models/Qwen3.5-4B-Uncensored-HauhauCS-Aggressive-Q4_K_M.gguf";
        vision_mmproj_path_ = "/sdcard/Download/Ai offline models/mmproj-Qwen3.5-4B-Uncensored-HauhauCS-Aggressive-BF16.gguf";
    }
    if (!img_path.empty() && !vision_core_path_.empty() && active_model_name_.find("Qwen3.5") == std::string::npos) {
        is_generating_ = false;
        lock.unlock();
        hot_swap_to_path(vision_core_path_);
        { std::ofstream vl("/sdcard/Izanami/memory/vision_log.txt", std::ios::app); vl << "swapped core=" << active_model_name_ << "\n"; }
        lock.lock();
        stop_flag_ = false;
        is_generating_ = true;
        if (!llama_model_ || !llama_context_) { is_generating_ = false; return; }
        vocab = llama_model_get_vocab(llama_model_);
    }
    if (!img_path.empty() && !mtmd_ctx_ && !vision_mmproj_path_.empty()) {
        current_status_ = "Loading eyes...";
        set_mmproj_path(vision_mmproj_path_);
        { std::ofstream vl("/sdcard/Izanami/memory/vision_log.txt", std::ios::app); vl << "mmproj loaded=" << (mtmd_ctx_ ? 1 : 0) << "\n"; }
    }

    if (!img_path.empty() && mtmd_ctx_) {
        remove("/sdcard/Izanami/memory/last_image.txt");
        mtmd_helper_bitmap_wrapper bw = mtmd_helper_bitmap_init_from_file(mtmd_ctx_, img_path.c_str(), false);
        mtmd_bitmap* bitmap = bw.bitmap;
        if (bitmap) {
            mtmd_input_chunks* chunks = mtmd_input_chunks_init();
            std::string final_prompt = std::string(mtmd_default_marker()) + "\n" + ptext;
            mtmd_input_text txt{final_prompt.c_str(), final_prompt.size(), true, true};
            const mtmd_bitmap* bitmaps[1] = {bitmap};
            int rc = mtmd_tokenize(mtmd_ctx_, chunks, &txt, bitmaps, 1);
            if (rc == 0) {
                llama_pos new_n_past = 0;
                int rc2 = mtmd_helper_eval_chunks(mtmd_ctx_, llama_context_, chunks, n_past, 0, 512, true, &new_n_past);
                if (rc2 == 0) n_past = new_n_past;
            }
            mtmd_input_chunks_free(chunks);
            mtmd_bitmap_free(bitmap);
        }
    } else {
        std::vector<llama_token> tokens(llama_n_ctx(llama_context_));
        int n_tokens = llama_tokenize(vocab, ptext.c_str(), ptext.size(), tokens.data(), tokens.size(), true, true);
        if (n_tokens < 0) {
            tokens.resize(-n_tokens);
            n_tokens = llama_tokenize(vocab, ptext.c_str(), ptext.size(), tokens.data(), tokens.size(), true, true);
        }
        tokens.resize(n_tokens);
        llama_batch batch = llama_batch_get_one(tokens.data(), tokens.size());
        if (llama_decode(llama_context_, batch) != 0) {
            current_status_ = "Decode Error";
            is_generating_ = false;
            return;
        }
        n_past = n_tokens;
    }

    llama_sampler* sampler = llama_sampler_chain_init(llama_sampler_chain_default_params());
    llama_sampler_chain_add(sampler, llama_sampler_init_greedy());

    while (true) {
        if (stop_flag_.load()) break;
        llama_token new_token_id = llama_sampler_sample(sampler, llama_context_, -1);
        if (new_token_id == llama_vocab_eos(vocab)) break;
        char buf[256];
        int n = llama_token_to_piece(vocab, new_token_id, buf, sizeof(buf), 0, true);
        if (n > 0) {
            std::string token_str(buf, n);
            current_status_ = "Generating...";
            if (!callback(token_str)) break;
        }
        llama_batch batch = llama_batch_get_one(&new_token_id, 1);
        if (llama_decode(llama_context_, batch) != 0) break;
    }

    llama_sampler_free(sampler);
    is_generating_ = false;
}

void InferenceEngine::update_ui_status(bool& model_loaded, std::string& status_message, std::string& active_model_name) {
    model_loaded = is_initialized_.load();
    status_message = current_status_;
    active_model_name = active_model_name_;
}

void InferenceEngine::hot_swap_to_path(const std::string& path) {
    if (path.empty()) return;
    std::string fname = path.substr(path.find_last_of("/") + 1);
    current_status_ = "Loading: " + fname.substr(0, 15) + "...";
    stop_generation();
    load_model_internal(path);
    active_model_name_ = fname;
    current_status_ = "Swapped to: " + fname;
    { std::ofstream pf("/sdcard/Izanami/memory/active_model.txt"); if (pf) pf << path; }
}
