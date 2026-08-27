#include "ModelRouter.hpp"
#include "Attach.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>

namespace ModelRouter {
    static bool picker_open_ = false;
    static double opened_at_ = 0.0;
    static std::vector<ModelInfo> models_;
    static std::string selected_path_;
    static const char* kModelDirs[2] = { "/sdcard/Download/Ai offline models/", "/sdcard/Izanami/models/" };

    void scan_models() {
        models_.clear();
        for (int d = 0; d < 2; ++d) {
            DIR* dh = opendir(kModelDirs[d]);
            if (!dh) continue;
            struct dirent* ent;
            while ((ent = readdir(dh)) != nullptr) {
                std::string name = ent->d_name;
                if (name.size() > 5 && name.substr(name.size() - 5) == ".gguf") {
                    struct stat st;
                    std::string full_path = std::string(kModelDirs[d]) + name;
                    if (stat(full_path.c_str(), &st) == 0) {
                        models_.push_back({name, full_path, st.st_size / (1024 * 1024)});
                    }
                }
            }
            closedir(dh);
        }
        std::sort(models_.begin(), models_.end(), [](const ModelInfo& a, const ModelInfo& b) {
            return a.filename < b.filename;
        });
    }

    void open_picker() { scan_models(); picker_open_ = true; opened_at_ = GetTime(); }
    void close_picker() { picker_open_ = false; }
    bool picker_open() { return picker_open_; }
    std::string get_selected_model_path() { return selected_path_; }
    std::string take_selected_path() { std::string p = selected_path_; selected_path_.clear(); return p; }
    std::vector<ModelInfo> get_available_models() { return models_; }

    void update_and_draw(int screen_width, int screen_height, int font_size) {
        if (!picker_open_) return;
        DrawRectangle(0, 0, screen_width, screen_height, (Color){0, 0, 0, 180});
        int panel_w = screen_width - 80;
        int row_h = 100;
        int rows = (int)models_.size() + 1;
        int panel_h = rows * row_h + 60;
        int px = 40, py = (screen_height - panel_h) / 2;
        DrawRectangle(px, py, panel_w, panel_h, (Color){15, 15, 15, 255});
        DrawRectangleLinesEx((Rectangle){(float)px, (float)py, (float)panel_w, (float)panel_h}, 2, (Color){180, 70, 255, 255});
        DrawText("SELECT MODEL CORE", px + 20, py + 15, font_size, (Color){180, 70, 255, 255});
        Vector2 mouse = GetMousePosition();
        bool pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && (GetTime() - opened_at_ > 0.3);
        for (int i = 0; i < (int)models_.size(); ++i) {
            Rectangle r = {(float)px + 10, (float)(py + 60 + i * row_h), (float)panel_w - 20, (float)row_h - 10};
            bool hov = CheckCollisionPointRec(mouse, r);
            if (hov) DrawRectangleRec(r, (Color){40, 40, 40, 255});
            DrawRectangleLinesEx(r, 1, (Color){180, 70, 255, 255});
            DrawText(models_[i].filename.c_str(), (int)r.x + 15, (int)r.y + 20, font_size, RAYWHITE);
            std::string size_str = std::to_string(models_[i].file_size_mb) + " MB";
            { std::string tag = "CHAT"; Color tcol = (Color){120, 255, 150, 255}; if (models_[i].filename.find("kokoro") != std::string::npos) { tag = "TTS"; tcol = (Color){255, 200, 80, 255}; } else if (models_[i].filename.find("stable-diffusion") != std::string::npos || models_[i].filename.find("anything") != std::string::npos) { tag = "IMG"; tcol = (Color){80, 180, 255, 255}; } DrawText((size_str + "  [" + tag + "]").c_str(), (int)r.x + 15, (int)r.y + 50, font_size - 5, tcol); }
            if (pressed && hov) {
                selected_path_ = models_[i].full_path;
                picker_open_ = false;
                return;
            }
        }
        Rectangle cancel = {(float)px + 10, (float)(py + 60 + (int)models_.size() * row_h), (float)panel_w - 20, (float)row_h - 10};
        bool hovc = CheckCollisionPointRec(mouse, cancel);
        if (hovc) DrawRectangleRec(cancel, (Color){40, 40, 40, 255});
        DrawRectangleLinesEx(cancel, 1, (Color){180, 70, 255, 255});
        DrawText("CANCEL", (int)cancel.x + 15, (int)cancel.y + 25, font_size, RAYWHITE);
        if (pressed && (hovc || !CheckCollisionPointRec(mouse, (Rectangle){(float)px, (float)py, (float)panel_w, (float)panel_h}))) {
            picker_open_ = false;
        }
    }
}
