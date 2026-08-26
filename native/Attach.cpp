#include "Attach.hpp"
#include "AndroidInput.hpp"
#include <fstream>
#include <cstdio>

namespace Attach {
    static std::string attached_path_;
    static std::string attached_name_;
    static Texture2D thumb_ = {0};
    static bool waiting_ = false;
    static double opened_at_ = 0.0;
    static const char* kMailbox = "/sdcard/Izanami/memory/picked_image.txt";

    void open_picker() { AndroidBridge::open_image_picker(); waiting_ = true; opened_at_ = GetTime(); }
    void close_picker() { waiting_ = false; }
    bool picker_open() { return waiting_; }
    bool has_attachment() { return !attached_path_.empty(); }
    std::string attachment_path() { return attached_path_; }
    std::string attachment_name() { return attached_name_; }
    void clear_attachment() { attached_path_.clear(); attached_name_.clear(); if (thumb_.id > 0) { UnloadTexture(thumb_); thumb_ = {0}; } }

    void update_and_draw(int sw, int sh, int font_size) {
        if (waiting_) {
            std::ifstream f(kMailbox);
            if (f) {
                std::string path; std::getline(f, path);
                f.close();
                if (!path.empty()) {
                    remove(kMailbox);
                    attached_path_ = path;
                    attached_name_ = "photo";
                    if (thumb_.id > 0) UnloadTexture(thumb_);
                    thumb_ = LoadTexture(path.c_str());
                    { std::ofstream lg("/sdcard/Izanami/memory/attach_log.txt", std::ios::app); lg << path << " id=" << thumb_.id << " " << thumb_.width << "x" << thumb_.height << "\n"; }
                    waiting_ = false;
                }
            }
            if (waiting_ && GetTime() - opened_at_ > 60.0) waiting_ = false;
        }
        if (!attached_path_.empty()) {
            int cw = 260, chh = 100;
            Rectangle chip = { 20, 420, (float)cw, (float)chh };
            DrawRectangleRec(chip, (Color){30,30,30,230});
            DrawRectangleLinesEx(chip, 2, (Color){180,70,255,255});
            if (thumb_.id > 0) {
                Rectangle src = {0,0,(float)thumb_.width,(float)thumb_.height};
                Rectangle dst = {chip.x+8, chip.y+8, 84, 84};
                DrawTexturePro(thumb_, src, dst, (Vector2){0,0}, 0, WHITE);
            } else {
                DrawRectangle((int)chip.x+8, (int)chip.y+8, 64, 64, (Color){60,20,90,255});
                DrawText("IMG", (int)chip.x+22, (int)chip.y+30, font_size, (Color){180,70,255,255});
            }
            DrawText("photo", (int)chip.x + 100, (int)chip.y + 38, font_size, RAYWHITE);
            Rectangle xbtn = { chip.x + cw - 48, chip.y + 32, 40, 40 };
            DrawText("X", (int)xbtn.x + 8, (int)xbtn.y + 6, font_size, (Color){255,90,90,255});
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), xbtn)) clear_attachment();
        }
    }
}
