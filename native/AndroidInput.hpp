#pragma once
#include <jni.h>
#include <string>

namespace AndroidBridge {
    JavaVM* get_jvm();
    jobject get_activity(JNIEnv* env);
    void open_image_picker();
}

namespace AndroidInput {
    void show_soft_keyboard();
    void hide_soft_keyboard();
    bool is_keyboard_visible();
    void haptic2(int milliseconds);
    void copy_to_clipboard(const std::string& text);
    void set_kb_visible(bool v);
    bool kb_visible();
    std::string get_clipboard_text();
}
