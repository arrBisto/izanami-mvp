#pragma once
#include "raylib.h"
#include <string>
#include <vector>

namespace ModelRouter {
    struct ModelInfo {
        std::string filename;
        std::string full_path;
        long file_size_mb;
    };
    
    void scan_models();
    void open_picker();
    void close_picker();
    bool picker_open();
    void update_and_draw(int screen_width, int screen_height, int font_size);
    std::string get_selected_model_path();
    std::string take_selected_path();
    std::vector<ModelInfo> get_available_models();
}
