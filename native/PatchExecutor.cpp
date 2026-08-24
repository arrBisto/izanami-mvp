#include "PatchExecutor.hpp"
#include "MemoryVault.hpp"
#include <vector>
#include <fstream>
#include <mutex>
#include <tuple>
#include <atomic>
#include <sys/stat.h>

namespace PatchExecutor {
    static std::mutex q_mutex_;
    static std::atomic<bool> busy_{false};

    void set_busy(bool b) { busy_ = b; }
    bool busy() { return busy_; }
    static std::vector<std::tuple<std::string, std::string, std::string>> draft_queue_;

    void queue_draft(const std::string& n, const std::string& d, const std::string& c) {
        std::string code = c;
        size_t sb = code.find("#!/");
        if (sb != std::string::npos) code = code.substr(sb);
        std::string clean;
        size_t pos = 0;
        while (pos != std::string::npos) {
            size_t nl = code.find('\n', pos);
            std::string line = code.substr(pos, nl == std::string::npos ? std::string::npos : nl - pos);
            if (line.rfind("```", 0) != 0) clean += line + "\n";
            pos = (nl == std::string::npos) ? std::string::npos : nl + 1;
        }
        std::lock_guard<std::mutex> l(q_mutex_);
        draft_queue_.push_back({n, d, clean});
    }

    void draft_dummy_patch(const std::string& name) {
        std::string desc = "AI-drafted patch for: " + name;
        std::string code = "#!/bin/bash\necho 'Hello from the Sanity Valve!' >> /sdcard/Izanami/memory/patch_test.log\necho 'Patch executed successfully.'";
        MemoryVault::get_instance().add_pending_patch(name, desc, code);
    }

    void update_and_draw(int screen_width, int screen_height, int font_size) {
        { std::lock_guard<std::mutex> l(q_mutex_); for (auto& t : draft_queue_) MemoryVault::get_instance().add_pending_patch(std::get<0>(t), std::get<1>(t), std::get<2>(t)); draft_queue_.clear(); }
        return;
        auto patches = MemoryVault::get_instance().get_pending_patches();
        if (patches.empty()) return;

        DrawRectangle(0, 0, screen_width, screen_height, (Color){0, 0, 0, 220});
        auto& p = patches[0];

        int panel_w = screen_width - 80;
        int panel_h = screen_height - 150;
        int px = 40, py = 75;

        DrawRectangle(px, py, panel_w, panel_h, (Color){20, 10, 10, 255});
        DrawRectangleLinesEx((Rectangle){(float)px, (float)py, (float)panel_w, (float)panel_h}, 3, (Color){255, 80, 80, 255});

        DrawText("SANITY VALVE: REVIEW PATCH", px + 20, py + 15, font_size, (Color){255, 80, 80, 255});
        DrawText(p.name.c_str(), px + 20, py + 50, font_size + 5, RAYWHITE);
        DrawText(p.description.c_str(), px + 20, py + 90, font_size, (Color){200, 200, 200, 255});

        DrawRectangle(px + 15, py + 130, panel_w - 30, 180, (Color){5, 5, 5, 255});
        DrawText(p.patch_code.c_str(), px + 25, py + 140, font_size - 2, (Color){120, 255, 120, 255});

        Rectangle btn_approve = {(float)(px + 20), (float)(py + panel_h - 80), (float)(panel_w / 2 - 30), 60};
        Rectangle btn_reject = {(float)(px + panel_w / 2 + 10), (float)(py + panel_h - 80), (float)(panel_w / 2 - 30), 60};

        DrawRectangleRec(btn_approve, (Color){40, 120, 40, 255});
        DrawText("APPROVE", (int)btn_approve.x + 40, (int)btn_approve.y + 20, font_size + 5, RAYWHITE);

        DrawRectangleRec(btn_reject, (Color){120, 40, 40, 255});
        DrawText("REJECT", (int)btn_reject.x + 40, (int)btn_reject.y + 20, font_size + 5, RAYWHITE);

        Vector2 mouse = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (CheckCollisionPointRec(mouse, btn_approve)) {
                mkdir("/sdcard/Izanami/forge", 0755);
                mkdir("/sdcard/Izanami/forge/patches", 0755);
                std::string path = "/sdcard/Izanami/forge/patches/patch_" + std::to_string(p.id) + ".sh";
                std::ofstream out(path);
                if (out) { out << p.patch_code; out.close(); }
                MemoryVault::get_instance().delete_pending_patch(p.id);
            } else if (CheckCollisionPointRec(mouse, btn_reject)) {
                MemoryVault::get_instance().delete_pending_patch(p.id);
            }
        }
    }
}
