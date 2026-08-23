#pragma once
#include "raylib.h"
#include <string>

class CustomKeypad {
public:
    static CustomKeypad& get_instance() {
        static CustomKeypad instance;
        return instance;
    }
    CustomKeypad(const CustomKeypad&) = delete;
    CustomKeypad& operator=(const CustomKeypad&) = delete;

    void update_and_draw(int screen_width, int screen_height, int font_size);
    float get_height(int screen_height) const;

private:
    CustomKeypad() = default;
    bool is_shifted_{false};
    bool is_symbols_{false};
    bool is_symbols2_{false};
    Font keypad_font_;
    bool font_loaded_{false};
    
    // Hold-to-repeat tracking
    int held_key_index_{-1};
    double hold_start_time_{0.0};
    double last_repeat_time_{0.0};
    
    // Returns 0 = no action, 1 = single tap, 2 = repeat action
    int draw_key(Rectangle bounds, const char* label, int font_size, bool is_special, int key_index);
};
