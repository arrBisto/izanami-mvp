#include "Keypad.hpp"
#include "ChatEngine.hpp"

int CustomKeypad::draw_key(Rectangle bounds, const char* label, int font_size, bool is_special, int key_index) {
    Color bg_color = (Color){10, 10, 10, 255};
    Color border_color = is_special ? (Color){120, 50, 180, 255} : (Color){180, 70, 255, 255};
    Color text_color = is_special ? (Color){180, 70, 255, 255} : WHITE;

    Vector2 mouse_pos = GetMousePosition();
    bool is_hovered = CheckCollisionPointRec(mouse_pos, bounds);
    int action = 0; // 0 = none, 1 = tapped, 2 = repeating

    if (is_hovered) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            action = 1;
            held_key_index_ = key_index;
            hold_start_time_ = GetTime();
            last_repeat_time_ = GetTime();
        } else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && held_key_index_ == key_index) {
            // If held for more than 0.4s, start repeating every 0.08s
            if (GetTime() - hold_start_time_ > 0.4) {
                if (GetTime() - last_repeat_time_ > 0.08) {
                    action = 2;
                    last_repeat_time_ = GetTime();
                }
            }
        }
    } else {
        if (held_key_index_ == key_index && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            held_key_index_ = -1;
        }
    }

    if (action > 0) {
        bg_color = (Color){180, 70, 255, 255}; // Highlight purple
        text_color = WHITE;
    }

    DrawRectangleRec(bounds, bg_color);
    DrawRectangleLinesEx(bounds, 1, border_color);
    
    Vector2 text_size = MeasureTextEx(GetFontDefault(), label, static_cast<float>(font_size), 1.0f);
    DrawText(label, 
             bounds.x + (bounds.width / 2) - (text_size.x / 2), 
             bounds.y + (bounds.height / 2) - (text_size.y / 2), 
             font_size, text_color);
             
    return action;
}

float CustomKeypad::get_height(int screen_height) const {
    return screen_height * 0.40f;
}

void CustomKeypad::update_and_draw(int screen_width, int screen_height, int font_size) {
    float keypad_height = get_height(screen_height);
    float padding = 5.0f;
    float y_start = screen_height - keypad_height;
    float row_h = (keypad_height - (padding * 6)) / 5.0f;

    DrawRectangle(0, y_start - 5, screen_width, keypad_height + 5, (Color){5, 5, 5, 255});

    int key_idx = 0; // Unique ID for hold tracking

    auto draw_full_row = [&](const char* chars, int num_keys, float y) {
        float gaps = num_keys + 1.0f;
        float avail_width = screen_width - (gaps * padding);
        float key_w = avail_width / num_keys;
        float x = padding;
        for (int i = 0; i < num_keys; ++i) {
            char str[2] = {chars[i], '\0'};
            Rectangle bounds = {x, y, key_w, row_h};
            if (draw_key(bounds, str, font_size, false, key_idx++)) {
                ChatEngine::get_instance().append_char(chars[i]);
                if (is_shifted_ && !is_symbols_) is_shifted_ = false;
            }
            x += key_w + padding;
        }
    };

    // Row 1
    float y1 = y_start + padding;
    const char* r1 = (is_shifted_ && !is_symbols_) ? "!@#$%^&*()" : "1234567890";
    draw_full_row(r1, 10, y1);

    // Row 2
    float y2 = y1 + row_h + padding;
    const char* r2 = is_symbols_ ? "-_=+[]{};:" : (is_shifted_ ? "QWERTYUIOP" : "qwertyuiop");
    draw_full_row(r2, 10, y2);

    // Row 3
    float y3 = y2 + row_h + padding;
    const char* r3 = is_symbols_ ? "'\".,<>/?~" : (is_shifted_ ? "ASDFGHJKL" : "asdfghjkl");
    draw_full_row(r3, 9, y3);

    // Row 4: 123, zxcvbnm, DEL
    float y4 = y3 + row_h + padding;
    {
        const char* r4 = is_symbols_ ? "!@#$%^&" : (is_shifted_ ? "ZXCVBNM" : "zxcvbnm");
        int num_letters = 7;
        float gaps = 10.0f;
        float avail_width = screen_width - (gaps * padding);
        float unit_w = avail_width / 10.0f;
        float letter_w = unit_w;
        float special_w = unit_w * 1.5f;

        float x = padding;
        Rectangle sym_bounds = {x, y4, special_w, row_h};
        const char* sym_label = is_symbols_ ? "ABC" : "123";
        if (draw_key(sym_bounds, sym_label, font_size, true, key_idx++)) {
            is_symbols_ = !is_symbols_;
            is_shifted_ = false;
        }

        x += special_w + padding;
        for (int i = 0; i < num_letters; ++i) {
            char str[2] = {r4[i], '\0'};
            Rectangle bounds = {x, y4, letter_w, row_h};
            if (draw_key(bounds, str, font_size, false, key_idx++)) {
                ChatEngine::get_instance().append_char(r4[i]);
                if (is_shifted_ && !is_symbols_) is_shifted_ = false;
            }
            x += letter_w + padding;
        }

        Rectangle bsp_bounds = {x, y4, special_w, row_h};
        if (draw_key(bsp_bounds, "DEL", font_size, true, key_idx++)) {
            ChatEngine::get_instance().backspace();
        }
    }

    // Row 5: Left, Right, Space, Enter
    float y5 = y4 + row_h + padding;
    {
        float gaps = 5.0f;
        float avail_width = screen_width - (gaps * padding);
        float unit_w = avail_width / 10.0f;
        float arrow_w = unit_w * 1.5f;
        float space_w = unit_w * 4.0f;
        float enter_w = unit_w * 3.0f;

        float x = padding;
        Rectangle left_bounds = {x, y5, arrow_w, row_h};
        if (draw_key(left_bounds, "<-", font_size, true, key_idx++)) {
            ChatEngine::get_instance().move_cursor_left();
        }

        x += arrow_w + padding;
        Rectangle space_bounds = {x, y5, space_w, row_h};
        if (draw_key(space_bounds, "SPACE", font_size, true, key_idx++)) {
            ChatEngine::get_instance().append_char(' ');
        }

        x += space_w + padding;
        Rectangle right_bounds = {x, y5, arrow_w, row_h};
        if (draw_key(right_bounds, "->", font_size, true, key_idx++)) {
            ChatEngine::get_instance().move_cursor_right();
        }

        x += arrow_w + padding;
        Rectangle enter_bounds = {x, y5, enter_w, row_h};
        if (draw_key(enter_bounds, "ENTER", font_size, true, key_idx++)) {
            ChatEngine::get_instance().submit();
        }
    }
}
