#include "raylib.h"
#include "InferenceEngine.hpp"
#include "LuaEngine.hpp"
#include "PackageManager.hpp"
#include "ChatEngine.hpp"
#include "Keypad.hpp"
#include <string>
#include <vector>

int get_scaled_font_size(float fraction) {
    return static_cast<int>(GetScreenWidth() * fraction);
}

int measure_wrapped_text_height(const char* text, int max_width, int font_size) {
    std::string str(text);
    std::string word;
    std::string line;
    int y = 0;
    for (size_t i = 0; i <= str.length(); ++i) {
        if (i == str.length() || str[i] == ' ' || str[i] == '\n') {
            std::string test_line = line + (line.empty() ? "" : " ") + word;
            if (MeasureText(test_line.c_str(), font_size) > max_width && !line.empty()) {
                y += font_size + 5;
                line = word;
            } else {
                line = test_line;
            }
            word.clear();
            if (i < str.length() && str[i] == '\n') {
                y += font_size + 5;
                line.clear();
            }
        } else {
            word += str[i];
        }
    }
    if (!line.empty()) {
        y += font_size + 5;
    }
    return y;
}

void draw_wrapped_text(const char* text, int x, int& y, int max_width, int font_size, Color color) {
    std::string str(text);
    std::string word;
    std::string line;
    for (size_t i = 0; i <= str.length(); ++i) {
        if (i == str.length() || str[i] == ' ' || str[i] == '\n') {
            std::string test_line = line + (line.empty() ? "" : " ") + word;
            if (MeasureText(test_line.c_str(), font_size) > max_width && !line.empty()) {
                DrawText(line.c_str(), x, y, font_size, color);
                y += font_size + 5;
                line = word;
            } else {
                line = test_line;
            }
            word.clear();
            if (i < str.length() && str[i] == '\n') {
                DrawText(line.c_str(), x, y, font_size, color);
                y += font_size + 5;
                line.clear();
            }
        } else {
            word += str[i];
        }
    }
    if (!line.empty()) {
        DrawText(line.c_str(), x, y, font_size, color);
        y += font_size + 5;
    }
}

void get_cursor_xy(const std::string& text, size_t cursor_pos, int x_start, int y_start, int max_width, int font_size, int& out_x, int& out_y) {
    std::string word;
    std::string line;
    int y = y_start;
    for (size_t i = 0; i < cursor_pos; ++i) {
        char c = text[i];
        if (c == ' ' || c == '\n') {
            std::string test_line = line + (line.empty() ? "" : " ") + word;
            if (MeasureText(test_line.c_str(), font_size) > max_width && !line.empty()) {
                y += font_size + 5;
                line = word;
            } else {
                line = test_line;
            }
            word.clear();
            if (c == '\n') {
                y += font_size + 5;
                line.clear();
            }
        } else {
            word += c;
        }
    }
    std::string test_line = line + (line.empty() ? "" : " ") + word;
    if (MeasureText(test_line.c_str(), font_size) > max_width && !line.empty()) {
        y += font_size + 5;
        out_x = x_start + MeasureText(word.c_str(), font_size);
    } else {
        out_x = x_start + MeasureText(test_line.c_str(), font_size);
    }
    out_y = y;
}

int main(void) {
    SetConfigFlags(FLAG_FULLSCREEN_MODE);
    InitWindow(0, 0, "Izanami");
    
    Color pure_black = { 0, 0, 0, 255 };
    Color izanami_purple = { 180, 70, 255, 255 };
    Color user_text_color = { 255, 255, 255, 255 };

    InferenceEngine::get_instance().start_initialization();
    LuaEngine::get_instance().initialize();
    PackageManager::get_instance().initialize();
    ChatEngine::get_instance();

    SetTargetFPS(60);
    bool is_input_active = false;
    float manual_scroll_offset = 0.0f; // For finger dragging

    while (!WindowShouldClose()) {
        int screen_width = GetScreenWidth();
        int screen_height = GetScreenHeight();
        int uniform_font_size = get_scaled_font_size(0.035f);
        int padding = get_scaled_font_size(0.02f);

        float input_box_height = uniform_font_size * 5.0f;
        float keypad_height = CustomKeypad::get_instance().get_height(screen_height);
        float ui_offset = is_input_active ? keypad_height : 0.0f;
        
        Rectangle input_box = { 
            (float)padding, 
            (float)(screen_height - input_box_height - padding - ui_offset), 
            (float)(screen_width - 2*padding), 
            input_box_height 
        };
        
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            bool tapped_input = CheckCollisionPointRec(GetMousePosition(), input_box);
            Rectangle keypad_area = {0, screen_height - keypad_height, (float)screen_width, keypad_height};
            bool tapped_keypad = CheckCollisionPointRec(GetMousePosition(), keypad_area);

            if (tapped_input) {
                is_input_active = true;
            } else if (!tapped_keypad) {
                is_input_active = false;
            }
        }

        BeginDrawing();
        ClearBackground(pure_black);

        bool model_loaded = false;
        std::string status_message = ""; 
        std::string active_model = "";
        InferenceEngine::get_instance().update_ui_status(model_loaded, status_message, active_model);

        // --- HEADER LAYOUT ---
        const char* title_text = "Izanami";
        int title_font_size = get_scaled_font_size(0.12f); 
        Vector2 title_size = MeasureTextEx(GetFontDefault(), title_text, static_cast<float>(title_font_size), 1.0f);
        int title_y = get_scaled_font_size(0.08f); 
        DrawText(title_text, 
                 static_cast<int>(screen_width / 2.0f - title_size.x / 2.0f), 
                 title_y, 
                 title_font_size, izanami_purple);

        Vector2 status_size = MeasureTextEx(GetFontDefault(), status_message.c_str(), static_cast<float>(uniform_font_size), 1.0f);
        int status_y = title_y + title_font_size + get_scaled_font_size(0.02f); 
        DrawText(status_message.c_str(), 
                 static_cast<int>(screen_width / 2.0f - status_size.x / 2.0f), 
                 status_y, 
                 uniform_font_size, RAYWHITE);

        int next_y = status_y + uniform_font_size + get_scaled_font_size(0.015f);

        if (model_loaded && !active_model.empty()) {
            std::string model_text = "Active Core: " + active_model;
            Vector2 model_size = MeasureTextEx(GetFontDefault(), model_text.c_str(), static_cast<float>(uniform_font_size), 1.0f);
            DrawText(model_text.c_str(), 
                     static_cast<int>(screen_width / 2.0f - model_size.x / 2.0f), 
                     next_y, 
                     uniform_font_size, izanami_purple);
                     
            next_y = next_y + uniform_font_size + get_scaled_font_size(0.015f);
        }

        std::string lua_text = "LuaJIT VM: Online";
        Vector2 lua_size = MeasureTextEx(GetFontDefault(), lua_text.c_str(), static_cast<float>(uniform_font_size), 1.0f);
        DrawText(lua_text.c_str(), 
                 static_cast<int>(screen_width / 2.0f - lua_size.x / 2.0f), 
                 next_y, 
                 uniform_font_size, RAYWHITE);

        // --- CHAT UI LAYOUT (WITH MANUAL & AUTO SCROLL) ---
        float chat_y = next_y + uniform_font_size + get_scaled_font_size(0.02f);
        float chat_area_bottom = input_box.y - padding;
        float chat_area_height = chat_area_bottom - chat_y;
        if (chat_area_height < 0) chat_area_height = 0;
        float chat_max_width = screen_width - 2 * padding;

        auto history = ChatEngine::get_instance().get_history();
        std::string streaming = ChatEngine::get_instance().get_streaming_response();
        
        int total_text_height = 0;
        for (const auto& msg : history) {
            std::string prefix = (msg.role == ChatRole::USER) ? "You: " : "Izanami: ";
            std::string line = prefix + msg.content;
            total_text_height += measure_wrapped_text_height(line.c_str(), chat_max_width, uniform_font_size);
        }
        if (!streaming.empty()) {
            std::string line = "Izanami: " + streaming;
            total_text_height += measure_wrapped_text_height(line.c_str(), chat_max_width, uniform_font_size);
        }

        // Base Y (bottom aligned if text is long, top aligned if short)
        float base_y = chat_y;
        if (total_text_height > chat_area_height) {
            base_y = chat_area_bottom - total_text_height - 10;
        }

        // Reset scroll to bottom if generating
        if (ChatEngine::get_instance().get_state() == ChatState::INFERRING) {
            manual_scroll_offset = 0.0f;
        }

        // Handle finger drag in chat area
        Rectangle chat_area = {static_cast<float>(padding), chat_y, chat_max_width, chat_area_height};
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), chat_area)) {
            manual_scroll_offset += GetMouseDelta().y;
        }

        float y_cursor = base_y + manual_scroll_offset;

        // Clamp scrolling (can't drag past top)
        if (y_cursor > chat_y) {
            y_cursor = chat_y;
            manual_scroll_offset = chat_y - base_y;
        }
        // Clamp scrolling (can't drag past bottom)
        if (total_text_height > chat_area_height && y_cursor + total_text_height < chat_area_bottom) {
            y_cursor = chat_area_bottom - total_text_height;
            manual_scroll_offset = y_cursor - base_y;
        }

        BeginScissorMode(padding, chat_y, chat_max_width, chat_area_height);
        
        int draw_y = static_cast<int>(y_cursor);
        for (const auto& msg : history) {
            std::string prefix = (msg.role == ChatRole::USER) ? "You: " : "Izanami: ";
            Color col = (msg.role == ChatRole::USER) ? user_text_color : izanami_purple;
            std::string line = prefix + msg.content;
            draw_wrapped_text(line.c_str(), padding, draw_y, chat_max_width, uniform_font_size, col);
        }
        if (!streaming.empty()) {
            std::string line = "Izanami: " + streaming;
            draw_wrapped_text(line.c_str(), padding, draw_y, chat_max_width, uniform_font_size, izanami_purple);
        }
        EndScissorMode();

        // Draw Input Box
        Color box_color = is_input_active ? (Color){30, 30, 30, 255} : (Color){20, 20, 20, 255};
        DrawRectangleRec(input_box, box_color);
        DrawRectangleLinesEx(input_box, 2, is_input_active ? izanami_purple : (Color){100, 100, 100, 255});
        
        std::string input_text = ChatEngine::get_instance().get_input();
        int input_text_y = input_box.y + 10;
        
        if (input_text.empty() && !is_input_active) {
            DrawText("Tap to chat...", padding + 5, input_text_y, uniform_font_size, (Color){100, 100, 100, 255});
        } else {
            BeginScissorMode(input_box.x, input_box.y, input_box.width, input_box.height);
            // Draw text and save the Y it ends on
            int draw_end_y = input_text_y;
            draw_wrapped_text(input_text.c_str(), padding + 5, draw_end_y, input_box.width - 20, uniform_font_size, RAYWHITE);
            
            if (is_input_active) {
                float blink_time = fmod(GetTime(), 1.0f);
                if (blink_time < 0.5f) {
                    int cursor_x = 0, cursor_y = 0;
                    // CRITICAL FIX: Use the original input_text_y, NOT draw_end_y
                    get_cursor_xy(input_text, ChatEngine::get_instance().get_cursor_pos(), padding + 5, input_text_y, input_box.width - 20, uniform_font_size, cursor_x, cursor_y);
                    DrawText("|", cursor_x, cursor_y, uniform_font_size, izanami_purple);
                }
            }
            EndScissorMode();
        }
        
        DrawText(ChatEngine::get_instance().get_status().c_str(), padding, input_box.y - uniform_font_size - 5, uniform_font_size, RAYWHITE);

        if (is_input_active) {
            CustomKeypad::get_instance().update_and_draw(screen_width, screen_height, uniform_font_size);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
