#include "TextWrap.hpp"
#include <string>

void draw_wrapped_text_v2(const char* text, int x, int& y, int max_width, int font_size, Color color) {
    std::string str(text);
    std::string word;
    std::string line;
    auto flush_line = [&]() {
        DrawText(line.c_str(), x, y, font_size, color);
        y += font_size + 5;
        line.clear();
    };
    auto append_word = [&]() {
        while (MeasureText(word.c_str(), font_size) > max_width && !word.empty()) {
            std::string piece;
            size_t i = 0;
            while (i < word.size()) {
                std::string test = piece + word[i];
                if (MeasureText(test.c_str(), font_size) > max_width) break;
                piece = test;
                ++i;
            }
            if (piece.empty()) { piece = word.substr(0, 1); i = 1; }
            DrawText(piece.c_str(), x, y, font_size, color);
            y += font_size + 5;
            word.erase(0, i);
        }
        std::string test_line = line + (line.empty() ? "" : " ") + word;
        if (MeasureText(test_line.c_str(), font_size) > max_width && !line.empty()) {
            flush_line();
            line = word;
        } else {
            line = test_line;
        }
        word.clear();
    };
    for (size_t i = 0; i <= str.length(); ++i) {
        if (i == str.length() || str[i] == ' ' || str[i] == '\n') {
            if (!word.empty()) append_word();
            if (i < str.length() && str[i] == '\n') flush_line();
        } else {
            word += str[i];
        }
    }
    if (!word.empty()) append_word();
    if (!line.empty()) flush_line();
}
