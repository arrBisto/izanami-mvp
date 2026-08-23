#pragma once
#include <jni.h>
#include <string>

namespace AndroidBridge {
    JavaVM* get_jvm();
    jobject get_activity(JNIEnv* env);
}

namespace AndroidInput {
    void show_soft_keyboard();
    void hide_soft_keyboard();
    bool is_keyboard_visible();
    void haptic2(int milliseconds);
    void copy_to_clipboard(const std::string& text);
    std::string get_clipboard_text();
}
