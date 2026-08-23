#include "Keypad.hpp"
#include "ChatEngine.hpp"
#include "AndroidInput.hpp"
#include <sys/stat.h>

int CustomKeypad::draw_key(Rectangle bounds, const char* label, int font_size, bool is_special, int key_index) {
    Color bg_color = (Color){0, 0, 0, 255};
    Color border_color = (Color){180, 70, 255, 255}; // Purple border always
    Color text_color = WHITE;

    Vector2 mouse_pos = GetMousePosition();
    bool is_hovered = CheckCollisionPointRec(mouse_pos, bounds);
    int action = 0;

    if (is_hovered) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            action = 1;
            AndroidInput::haptic2(80); // Haptic feedback on press
            held_key_index_ = key_index;
            hold_start_time_ = GetTime();
            last_repeat_time_ = GetTime();
        } else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && held_key_index_ == key_index) {
            if (GetTime() - hold_start_time_ > 0.4) {
                if (GetTime() - last_repeat_time_ > 0.08) {
                    action = 2;
                    last_repeat_time_ = GetTime();
                }
            }
        }
    } else {
        if (held_key_index_ == key_index && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) held_key_index_ = -1;
    }

    if (action > 0) bg_color = (Color){30, 30, 30, 255}; // Highlight on press

    DrawRectangleRec(bounds, bg_color);
    DrawRectangleLinesEx(bounds, 2, border_color); // Thick purple box

    Font f = font_loaded_ ? keypad_font_ : GetFontDefault();
    Vector2 text_size = MeasureTextEx(f, label, (float)font_size, 1.5f);
    Vector2 pos = {
        bounds.x + (bounds.width / 2) - (text_size.x / 2),
        bounds.y + (bounds.height / 2) - (text_size.y / 2)
    };
    DrawTextEx(f, label, pos, (float)font_size, 1.5f, text_color);

    return action;
}

float CustomKeypad::get_height(int screen_height) const {
    return screen_height * 0.42f;
}
void CustomKeypad::update_and_draw(int screen_width, int screen_height, int font_size) {
    if (!font_loaded_) {
        struct stat st;
        if (stat("/system/fonts/Roboto-Bold.ttf", &st) == 0) {
            keypad_font_ = LoadFontEx("/system/fonts/Roboto-Bold.ttf", 64, 0, 250);
            font_loaded_ = true;
        } else {
            keypad_font_ = GetFontDefault();
            font_loaded_ = true;
        }
    }

    float keypad_height = get_height(screen_height);
    float padding = 6.0f;
    float y_start = screen_height - keypad_height;
    float row_h = (keypad_height - (padding * 5)) / 4.0f;

    DrawRectangle(0, y_start, screen_width, keypad_height, (Color){0, 0, 0, 255});

    int key_idx = 0;
    float unit_w = (screen_width - (11 * padding)) / 10.0f;

    // Row 1 (10 keys)
    float y1 = y_start + padding;
    const char* r1 = (is_shifted_ && !is_symbols_) ? "!@#$%^&*()" : (is_symbols_ ? "1234567890" : "qwertyuiop");
    float x = padding;
    for (int i = 0; i < 10; ++i) {
        char str[2] = {r1[i], '\0'};
        Rectangle b = {x, y1, unit_w, row_h};
        if (draw_key(b, str, font_size, false, key_idx++)) {
            ChatEngine::get_instance().append_char(r1[i]);
            if (is_shifted_ && !is_symbols_) is_shifted_ = false;
        }
        x += unit_w + padding;
    }

    // Row 2 (9 keys)
    float y2 = y1 + row_h + padding;
    const char* r2 = is_symbols_ ? "-/:;()$&@\"" : (is_shifted_ ? "ASDFGHJKL" : "asdfghjkl");
    float x2_start = padding + (unit_w * 0.5f) + (padding * 0.5f);
    x = x2_start;
    for (int i = 0; i < 9; ++i) {
        char str[2] = {r2[i], '\0'};
        Rectangle b = {x, y2, unit_w, row_h};
        if (draw_key(b, str, font_size, false, key_idx++)) {
            ChatEngine::get_instance().append_char(r2[i]);
            if (is_shifted_ && !is_symbols_) is_shifted_ = false;
        }
        x += unit_w + padding;
    }

    // Row 3 (Shift + 7 keys + Backspace)
    float y3 = y2 + row_h + padding;
    float shift_w = unit_w * 1.5f;
    float bsp_w = unit_w * 1.5f;
    x = padding;
    Rectangle shift_b = {x, y3, shift_w, row_h};
    if (draw_key(shift_b, is_symbols_ ? "1/2" : "SHIFT", (int)(font_size * 0.7f), true, key_idx++)) {
        if (is_symbols_) is_symbols2_ = !is_symbols2_; // Next phase feature hook
        else is_shifted_ = !is_shifted_;
    }
    x += shift_w + padding;
    const char* r3 = is_symbols_ ? "%&*+-=()$" : (is_shifted_ ? "ZXCVBNM" : "zxcvbnm");
    for (int i = 0; i < 7; ++i) {
        char str[2] = {r3[i], '\0'};
        Rectangle b = {x, y3, unit_w, row_h};
        if (draw_key(b, str, font_size, false, key_idx++)) {
            ChatEngine::get_instance().append_char(r3[i]);
            if (is_shifted_ && !is_symbols_) is_shifted_ = false;
        }
        x += unit_w + padding;
    }
    Rectangle bsp_b = {x, y3, bsp_w, row_h};
    if (draw_key(bsp_b, "DEL", (int)(font_size * 0.8f), true, key_idx++)) {
        ChatEngine::get_instance().backspace();
    }

    // Row 4 (Sym + Comma + Space + Dot + Enter)
    float y4 = y3 + row_h + padding;
    float sym_w = unit_w * 1.5f;
    float space_w = unit_w * 5.0f;
    float enter_w = unit_w * 1.5f;
    x = padding;
    Rectangle sym_b = {x, y4, sym_w, row_h};
    if (draw_key(sym_b, is_symbols_ ? "ABC" : "?123", (int)(font_size * 0.8f), true, key_idx++)) {
        is_symbols_ = !is_symbols_;
        is_shifted_ = false;
    }
    x += sym_w + padding;
    Rectangle comma_b = {x, y4, unit_w, row_h};
    if (draw_key(comma_b, is_symbols_ ? "<" : ",", font_size, false, key_idx++)) {
        ChatEngine::get_instance().append_char(is_symbols_ ? '<' : ',');
    }
    x += unit_w + padding;
    Rectangle space_b = {x, y4, space_w, row_h};
    if (draw_key(space_b, "SPACE", (int)(font_size * 0.8f), true, key_idx++)) {
        ChatEngine::get_instance().append_char(' ');
    }
    x += space_w + padding;
    Rectangle dot_b = {x, y4, unit_w, row_h};
    if (draw_key(dot_b, is_symbols_ ? ">" : ".", font_size, false, key_idx++)) {
        ChatEngine::get_instance().append_char(is_symbols_ ? '>' : '.');
    }
    x += unit_w + padding;
    Rectangle enter_b = {x, y4, enter_w, row_h};
    if (draw_key(enter_b, "ENTER", (int)(font_size * 0.8f), true, key_idx++)) {
        ChatEngine::get_instance().submit();
    }
}
