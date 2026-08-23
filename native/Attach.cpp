#include "Attach.hpp"
#include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include <string>
#include <algorithm>

namespace Attach {
    static bool picker_open_ = false;
    static double opened_at_ = 0.0;
    static std::string attached_path_;
    static std::string attached_name_;
    static std::vector<std::string> items_;
    static const char* kDir = "/sdcard/Pictures/Screenshots/";

    static void refresh() {
        items_.clear();
        DIR* d = opendir(kDir);
        if (!d) return;
        struct dirent* ent;
        std::vector<std::pair<time_t, std::string>> files;
        while ((ent = readdir(d)) != nullptr) {
            std::string name = ent->d_name;
            if (name.size() > 4 && (name.substr(name.size() - 4) == ".png" || name.substr(name.size() - 4) == ".jpg")) {
                struct stat st;
                if (stat((std::string(kDir) + name).c_str(), &st) == 0) files.push_back({st.st_mtime, name});
            }
        }
        closedir(d);
        std::sort(files.begin(), files.end(), [](const std::pair<time_t, std::string>& a, const std::pair<time_t, std::string>& b) { return a.first > b.first; });
        for (size_t i = 0; i < files.size() && i < 5; ++i) items_.push_back(files[i].second);
    }

    void open_picker() { refresh(); picker_open_ = true; opened_at_ = GetTime(); }
    void close_picker() { picker_open_ = false; }
    bool picker_open() { return picker_open_; }
    bool has_attachment() { return !attached_path_.empty(); }
    std::string attachment_path() { return attached_path_; }
    std::string attachment_name() { return attached_name_; }
    void clear_attachment() { attached_path_.clear(); attached_name_.clear(); }

    void update_and_draw(int screen_width, int screen_height, int font_size) {
        if (!picker_open_) return;
        DrawRectangle(0, 0, screen_width, screen_height, (Color){0, 0, 0, 180});
        int panel_w = screen_width - 80;
        int row_h = 90;
        int rows = (int)items_.size() + 1;
        int panel_h = rows * row_h + 60;
        int px = 40, py = (screen_height - panel_h) / 2;
        DrawRectangle(px, py, panel_w, panel_h, (Color){15, 15, 15, 255});
        DrawRectangleLinesEx((Rectangle){(float)px, (float)py, (float)panel_w, (float)panel_h}, 2, (Color){180, 70, 255, 255});
        DrawText("ATTACH SCREENSHOT", px + 20, py + 15, font_size, (Color){180, 70, 255, 255});
        Vector2 mouse = GetMousePosition();
        bool pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && (GetTime() - opened_at_ > 0.3);
        for (int i = 0; i < (int)items_.size(); ++i) {
            Rectangle r = {(float)px + 10, (float)(py + 60 + i * row_h), (float)panel_w - 20, (float)row_h - 10};
            bool hov = CheckCollisionPointRec(mouse, r);
            if (hov) DrawRectangleRec(r, (Color){40, 40, 40, 255});
            DrawRectangleLinesEx(r, 1, (Color){180, 70, 255, 255});
            DrawText(items_[i].c_str(), (int)r.x + 15, (int)r.y + 25, font_size, RAYWHITE);
            if (pressed && hov) {
                attached_name_ = items_[i];
                attached_path_ = std::string(kDir) + items_[i];
                picker_open_ = false;
                return;
            }
        }
        Rectangle cancel = {(float)px + 10, (float)(py + 60 + (int)items_.size() * row_h), (float)panel_w - 20, (float)row_h - 10};
        bool hovc = CheckCollisionPointRec(mouse, cancel);
        if (hovc) DrawRectangleRec(cancel, (Color){40, 40, 40, 255});
        DrawRectangleLinesEx(cancel, 1, (Color){180, 70, 255, 255});
        DrawText("CANCEL", (int)cancel.x + 15, (int)cancel.y + 25, font_size, RAYWHITE);
        if (pressed && (hovc || !CheckCollisionPointRec(mouse, (Rectangle){(float)px, (float)py, (float)panel_w, (float)panel_h}))) {
            picker_open_ = false;
        }
    }
}
