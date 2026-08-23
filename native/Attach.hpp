#pragma once
#include "raylib.h"
#include <string>
namespace Attach {
    void open_picker();
    void close_picker();
    bool picker_open();
    void update_and_draw(int screen_width, int screen_height, int font_size);
    bool has_attachment();
    std::string attachment_path();
    std::string attachment_name();
    void clear_attachment();
}
