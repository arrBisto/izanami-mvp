#pragma once
#include "raylib.h"
#include <string>

namespace PatchExecutor {
    void update_and_draw(int screen_width, int screen_height, int font_size);
    void draft_dummy_patch(const std::string& name);
    void queue_draft(const std::string& n, const std::string& d, const std::string& c);
    void set_busy(bool b);
    bool busy();
}
