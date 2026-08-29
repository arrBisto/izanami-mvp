#include "raylib.h"
#include "InferenceEngine.hpp"
#include "LuaEngine.hpp"
#include "PackageManager.hpp"
#include "ChatEngine.hpp"
#include "Keypad.hpp"
#include "AndroidInput.hpp"
#include "Attach.hpp"
#include "ModelRouter.hpp"
#include "PatchExecutor.hpp"
#include <thread>
#include "CrashGuard.hpp"
#include "MemoryVault.hpp"
#include "Forge.hpp"
#include "Avatar.hpp"

static AvatarModel g_stage_avatar;
static bool stage_on = false;
#include "TextWrap.hpp"
#include <vector>
#include <string>
struct MessageRect { int y_top; int y_bottom; std::string content; };
static std::vector<MessageRect> g_message_rects;
static std::string g_flash_text;
static double g_flash_until = 0.0;
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


#include <vector>
#include <cmath>
void RunSplashScreen() {
    double start_time = GetTime();
    bool can_dismiss = false;
    int sw = GetScreenWidth(), sh = GetScreenHeight();
    std::vector<int> cpv;
    for (int c = 0x30A0; c <= 0x30FF; c++) cpv.push_back(c);
    Font jp = LoadFontEx("NotoSansJP-Bold.ttf", 48, cpv.data(), (int)cpv.size());
    bool have_jp = (jp.glyphCount > 50);
    if (!have_jp) { UnloadFont(jp); jp = LoadFontEx("NotoSansJP-Regular.ttf", 48, cpv.data(), (int)cpv.size()); have_jp = (jp.glyphCount > 50); }
    Font rf = have_jp ? jp : GetFontDefault();
    std::vector<std::string> glyphs;
    if (have_jp) {
        for (int c = 0x30A0; c <= 0x30FF; c++) {
            std::string u;
            u += (char)(0xE0 | (c >> 12));
            u += (char)(0x80 | ((c >> 6) & 63));
            u += (char)(0x80 | (c & 63));
            glyphs.push_back(u);
        }
    } else {
        for (const char* s2 : {"0","1","<",">","{","}","#","*","+","I","Z","A","N","M"}) glyphs.push_back(s2);
    }
    int cell = 36;
    int trail = 45;
    int cols = sw / cell + 1;
    std::vector<float> hy(cols), sp(cols);
    std::vector<int> gi(cols);
    for (int i = 0; i < cols; i++) { hy[i] = -((float)(rand() % sh)); sp[i] = 2.0f + (float)(rand() % 40) / 10.0f; gi[i] = rand() % (int)glyphs.size(); }
    Texture2D logo = LoadTexture("izanami_logo2.png");
    float cx = sw / 2.0f, cyc = sh / 2.0f - 30.0f;
    float R = (sw < sh ? sw : sh) * 0.30f;
    float fs = (float)(cell + 10);
    while (!WindowShouldClose()) {
        double el = GetTime() - start_time;
        if (el > 3.0) can_dismiss = true;
        if (can_dismiss && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) break;
        BeginDrawing();
        ClearBackground((Color){0,0,0,255});
        for (int i = 0; i < cols; i++) {
            float x = i * cell;
            int yhead = (int)hy[i];
            for (int t = 0; t < trail; t++) {
                int y = yhead - t * cell;
                if (y < -cell) break;
                if (y > sh + cell) continue;
                float dx = x - cx, dy = y - cyc;
                if (dx*dx + dy*dy < (R+20)*(R+20)) continue;
                const std::string& one = glyphs[(gi[i] + t * 13) % (int)glyphs.size()];
                Color col;
                if (t == 0) col = (Color){255,255,255,255};
                else if (t < 3) col = (Color){230,200,255,250};
                else if (t < 8) col = (Color){190,150,240,220};
                else if (t < 16) col = (Color){160,120,220,190};
                else col = (Color){130,95,200,160};
                DrawTextEx(rf, one.c_str(), (Vector2){x, (float)y}, fs, 0, col);
                DrawTextEx(rf, one.c_str(), (Vector2){x + 2, (float)y}, fs, 0, col);
            }
            hy[i] += sp[i];
            if (hy[i] - trail * cell > sh) { hy[i] = -(float)(rand() % 150); sp[i] = 2.0f + (float)(rand() % 40) / 10.0f; gi[i] = rand() % (int)glyphs.size(); }
            if (rand() % 60 == 0) gi[i] = rand() % (int)glyphs.size();
        }
        float pulse = 0.5f + 0.5f * sinf((float)el * 2.4f);
        DrawCircle((int)cx, (int)cyc, (int)R, (Color){0,0,0,255});
        for (int k = 0; k < 40; k++) {
            float a = (1.0f - k / 40.0f);
            a = a * a * (0.55f + 0.45f * pulse);
            DrawCircleLines((int)cx, (int)cyc, R + k * 1.2f, (Color){210,130,255,(unsigned char)(a * 180)});
        }
        for (int k = 0; k < 24; k++) {
            float a = (1.0f - k / 24.0f);
            a = a * a * (0.30f + 0.25f * pulse);
            DrawCircleLines((int)cx, (int)cyc, R + 48 + k * 2.0f, (Color){180,80,255,(unsigned char)(a * 130)});
        }
        for (int k = 0; k < 10; k++) {
            float a = (1.0f - k / 10.0f) * (0.25f + 0.20f * pulse);
            DrawCircleLines((int)cx, (int)cyc, R - k * 1.5f, (Color){210,130,255,(unsigned char)(a * 100)});
        }
        float name_y = cyc + R * 0.55f;
        if (logo.id > 0) {
            float aspect = (float)logo.width / (float)logo.height;
            int lh = (int)(R * 1.45f);
            int lw = (int)(lh * aspect);
            if (lw > (int)(R * 1.5f)) { lw = (int)(R * 1.5f); lh = (int)(lw / aspect); }
            float bottom = cyc + R * 0.50f;
            DrawTexturePro(logo, (Rectangle){0,0,(float)logo.width,(float)logo.height}, (Rectangle){cx - lw/2.0f, bottom - lh, (float)lw, (float)lh}, (Vector2){0,0}, 0.0f, WHITE);
        }
        const char* name = "Izanami";
        int ns = (int)(R * 0.26f);
        int nw = MeasureText(name, ns);
        float wx = 1.0f + pulse * 1.5f;
        DrawText(name, (int)(cx - nw/2) - (int)wx, (int)name_y, ns, (Color){255,255,255,(unsigned char)(90 + pulse*120)});
        DrawText(name, (int)(cx - nw/2) + (int)wx, (int)name_y, ns, (Color){255,255,255,(unsigned char)(90 + pulse*120)});
        DrawText(name, (int)(cx - nw/2), (int)name_y - (int)wx, ns, (Color){255,255,255,(unsigned char)(90 + pulse*120)});
        DrawText(name, (int)(cx - nw/2), (int)name_y + (int)wx, ns, (Color){255,255,255,(unsigned char)(90 + pulse*120)});
        DrawText(name, (int)(cx - nw/2), (int)name_y, ns, (Color){180,70,255,255});
        if (can_dismiss) {
            const char* tap = "TAP TO ENTER";
            int ts = (int)(R * 0.16f);
            int tw = MeasureText(tap, ts);
            float tx = (sw - tw) / 2.0f, ty = (float)(sh - 90);
            DrawText(tap, (int)tx - 1, (int)ty, ts, (Color){255,255,255,(unsigned char)(80 + pulse*100)});
            DrawText(tap, (int)tx + 1, (int)ty, ts, (Color){255,255,255,(unsigned char)(80 + pulse*100)});
            DrawText(tap, (int)tx, (int)ty, ts, (Color){180,70,255,255});
        }
        EndDrawing();
    }
    if (logo.id > 0) UnloadTexture(logo);
    if (have_jp) UnloadFont(jp);
}

int main(void) {
    bool previous_crashed = CrashGuard::PreviousSessionCrashed();
    std::string crash_report_path = "";
    if (previous_crashed) crash_report_path = CrashGuard::GenerateCrashReport();
    SetConfigFlags(FLAG_FULLSCREEN_MODE);
    InitWindow(0, 0, "Izanami");
    
    Color pure_black = { 0, 0, 0, 255 };
    Color izanami_purple = { 180, 70, 255, 255 };
    Color user_text_color = { 255, 255, 255, 255 };

    InferenceEngine::get_instance().start_initialization();
    LuaEngine::get_instance().initialize();
    PackageManager::get_instance().initialize();
    ChatEngine::get_instance();
    CrashGuard::Initialize();
    Avatar::load(g_stage_avatar, "/sdcard/Download/vrm models/chisa gltf/scene.gltf", "chisa");
    Avatar::upload(g_stage_avatar);
    MemoryVault::get_instance();
    CrashGuard::ArmWatchdog(90);
    if (previous_crashed) {
        ChatEngine::get_instance().add_message(ChatRole::SYSTEM,
            "INCIDENT DETECTED: Previous session terminated by a fatal signal. Black-box report saved to " + crash_report_path + ". Cross-reference the Gotcha Ledger and propose a fix.");
    }

    SetTargetFPS(60);
    RunSplashScreen();
    bool is_input_active = false;
    float manual_scroll_offset = 0.0f; // For finger dragging

    while (!WindowShouldClose()) {
        static double last_watchdog = 0.0;
        if (GetTime() - last_watchdog > 30.0) { CrashGuard::ArmWatchdog(90); last_watchdog = GetTime(); }
        static double last_forge_poll = 0.0;
        if (GetTime() - last_forge_poll > 1.0) {
            last_forge_poll = GetTime();
            std::string forge_result = Forge::get_instance().PollResult();
            if (!forge_result.empty()) ChatEngine::get_instance().add_message(ChatRole::ASSISTANT, forge_result);
        }
        if (stage_on) {
            static Vector2 sdown = {-1, -1};
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) sdown = GetMousePosition();
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                if (sdown.x >= 0) {
                    Vector2 t = GetMousePosition();
                    float dx = t.x - sdown.x, dy = t.y - sdown.y;
                    if (dx > 90 && fabsf(dy) < 60) stage_on = false;
                }
                sdown = {-1, -1};
            }
            BeginDrawing();
            ClearBackground(BLACK);
            Camera3D cam = {{0.0f, 1.15f, 2.4f}, {0.0f, 0.9f, 0.0f}, {0.0f, 1.0f, 0.0f}, 50.0f, CAMERA_PERSPECTIVE};
            BeginMode3D(cam);
            Avatar::draw(g_stage_avatar);
            EndMode3D();
            DrawText("< swipe right to return", 12, 12, 20, (Color){180, 70, 255, 255});
            EndDrawing();
            continue;
        }
        int screen_width = GetScreenWidth();
        int screen_height = GetScreenHeight();
        int uniform_font_size = get_scaled_font_size(0.035f);
    int chat_font_size = get_scaled_font_size(0.05f);
        int padding = get_scaled_font_size(0.02f);

        float input_box_height = chat_font_size * 2.6f;
        float keypad_height = CustomKeypad::get_instance().get_height(screen_height);
        float ui_offset = (is_input_active || AndroidInput::kb_visible()) ? screen_height * 0.36f : 0.0f;
        
        Rectangle input_box = { 
            (float)padding, 
            (float)(screen_height - input_box_height - padding - ui_offset), 
            (float)(screen_width - 2*padding), 
            input_box_height 
        };
        
        {
            static Vector2 cdown = {-1, -1};
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) cdown = GetMousePosition();
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                if (cdown.x >= 0) {
                    Vector2 t = GetMousePosition();
                    float dx = t.x - cdown.x, dy = t.y - cdown.y;
                    if (dx < -90 && fabsf(dy) < 60) stage_on = true;
                }
                cdown = {-1, -1};
            }
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            bool tapped_input = CheckCollisionPointRec(GetMousePosition(), input_box);
            Rectangle keypad_area = {0, screen_height - keypad_height, (float)screen_width, keypad_height};
            bool tapped_keypad = CheckCollisionPointRec(GetMousePosition(), keypad_area);

            if (tapped_input) {
                if (!is_input_active) AndroidInput::show_soft_keyboard();
                is_input_active = true;
            } else if (!tapped_keypad) {
                if (is_input_active) AndroidInput::hide_soft_keyboard();
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
            total_text_height += measure_wrapped_text_height(line.c_str(), chat_max_width, chat_font_size);
        }
        if (!streaming.empty()) {
            std::string line = "Izanami: " + streaming;
            total_text_height += measure_wrapped_text_height(line.c_str(), chat_max_width, chat_font_size);
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

        {
            static double press_start = 0.0;
            static bool tracking = false;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), chat_area)) { press_start = GetTime(); tracking = true; }
            if (tracking && IsMouseButtonDown(MOUSE_BUTTON_LEFT) && GetTime() - press_start > 2.0) {
                Vector2 mp = GetMousePosition();
                for (const auto& mr : g_message_rects) {
                    if (mp.y >= mr.y_top && mp.y <= mr.y_bottom) {
                        AndroidInput::copy_to_clipboard(mr.content);
                        g_flash_text = "Copied to clipboard";
                        g_flash_until = GetTime() + 1.5;
                        break;
                    }
                }
                tracking = false;
            }
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) tracking = false;
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
        
        g_message_rects.clear();
        int draw_y = static_cast<int>(y_cursor);
        for (const auto& msg : history) {
            std::string prefix = (msg.role == ChatRole::USER) ? "You: " : "Izanami: ";
            Color col = (msg.role == ChatRole::USER) ? user_text_color : izanami_purple;
            std::string line = prefix + msg.content;
            { int y0 = draw_y; draw_wrapped_text_v2(line.c_str(), padding, draw_y, chat_max_width, chat_font_size, col); g_message_rects.push_back({y0, draw_y, msg.content}); }
        }
        if (!streaming.empty()) {
            std::string line = "Izanami: " + streaming;
            draw_wrapped_text_v2(line.c_str(), padding, draw_y, chat_max_width, chat_font_size, izanami_purple);
        }
        EndScissorMode();
        if (g_flash_until > GetTime()) DrawText(g_flash_text.c_str(), padding, (int)chat_y + 5, chat_font_size, izanami_purple);

        // Draw Input Box
        Rectangle plus_btn = {(float)(screen_width - 110), input_box.y - 92, 90, 72};
        DrawRectangleRec(plus_btn, (Color){10, 10, 10, 255});
        DrawRectangleLinesEx(plus_btn, 2, izanami_purple);
        { Vector2 ps = MeasureTextEx(GetFontDefault(), "+", (float)(uniform_font_size + 16), 1.0f);
          int gx = (int)(plus_btn.x + plus_btn.width / 2 - ps.x / 2);
          int gy = (int)(plus_btn.y + plus_btn.height / 2 - ps.y / 2);
          DrawText("+", gx - 1, gy, uniform_font_size + 16, izanami_purple);
          DrawText("+", gx + 1, gy, uniform_font_size + 16, izanami_purple);
          DrawText("+", gx, gy - 1, uniform_font_size + 16, izanami_purple);
          DrawText("+", gx, gy + 1, uniform_font_size + 16, izanami_purple);
          DrawText("+", gx, gy, uniform_font_size + 16, izanami_purple); }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), plus_btn)) Attach::open_picker();
        if (Attach::has_attachment()) {
            Rectangle chip = {(float)padding, input_box.y - 70, 500, 60};
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), chip)) Attach::clear_attachment();
        }
        Color box_color = is_input_active ? (Color){30, 30, 30, 255} : (Color){20, 20, 20, 255};
        DrawRectangleRec(input_box, box_color);
        DrawRectangleLinesEx(input_box, 2, is_input_active ? izanami_purple : (Color){100, 100, 100, 255});
        
        std::string input_text = ChatEngine::get_instance().get_input();
        int input_text_y = input_box.y + 10;
        
        if (input_text.empty() && !is_input_active) {
            DrawText("Tap to chat...", padding + 5, input_text_y, chat_font_size, (Color){100, 100, 100, 255});
        } else {
            BeginScissorMode(input_box.x, input_box.y, input_box.width, input_box.height);
            // Draw text and save the Y it ends on
            int total_input_h = measure_wrapped_text_height(input_text.c_str(), (int)input_box.width - 20, chat_font_size);
            int visible_h = (int)input_box.height - 20;
            int scroll_start_y = (total_input_h > visible_h) ? input_text_y - (total_input_h - visible_h) : input_text_y;
            int draw_end_y = scroll_start_y;
            draw_wrapped_text_v2(input_text.c_str(), padding + 5, draw_end_y, input_box.width - 20, chat_font_size, RAYWHITE);
            
            if (is_input_active) {
                float blink_time = fmod(GetTime(), 1.0f);
                if (blink_time < 0.5f) {
                    int cursor_x = 0, cursor_y = 0;
                    // CRITICAL FIX: Use the original input_text_y, NOT draw_end_y
                    get_cursor_xy(input_text, ChatEngine::get_instance().get_cursor_pos(), padding + 5, scroll_start_y, input_box.width - 20, chat_font_size, cursor_x, cursor_y);
                    DrawText("|", cursor_x, cursor_y, uniform_font_size, izanami_purple);
                }
            }
            EndScissorMode();
        if (g_flash_until > GetTime()) DrawText(g_flash_text.c_str(), padding, (int)chat_y + 5, chat_font_size, izanami_purple);
        }
        
        DrawText(ChatEngine::get_instance().get_status().c_str(), padding, input_box.y - uniform_font_size - 5, uniform_font_size, RAYWHITE);

        if (is_input_active) {
            // System IME mode - custom keypad retired (kept as fallback in Keypad.cpp)
        }

        if (ChatEngine::get_instance().take_models_request()) ModelRouter::open_picker();
        std::string swap_path = ModelRouter::take_selected_path();
        if (!swap_path.empty()) {
            std::thread([swap_path]() { InferenceEngine::get_instance().hot_swap_to_path(swap_path); }).detach();
        }
        PatchExecutor::update_and_draw(screen_width, screen_height, uniform_font_size);
        ModelRouter::update_and_draw(screen_width, screen_height, uniform_font_size);
        Attach::update_and_draw(screen_width, screen_height, uniform_font_size);
        EndDrawing();
    }

    ChatEngine::get_instance().stop_inference();
    CrashGuard::ShutdownClean();
    CloseWindow();
    return 0;
}
