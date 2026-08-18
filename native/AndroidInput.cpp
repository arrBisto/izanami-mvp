#include "AndroidInput.hpp"
#include "raylib.h"
#include <android/native_activity.h>
#include <android_native_app_glue.h>

static bool keyboard_visible = false;
static ANativeActivity* get_native_activity() {
    struct android_app* app = static_cast<struct android_app*>(GetWindowHandle());
    if (!app || !app->activity) return nullptr;
    return app->activity;
}

void AndroidInput::show_soft_keyboard() {
    ANativeActivity* activity = get_native_activity();
    if (!activity) return;
    
    // Official NDK call. Talks directly to the OS Window Manager.
    // ANATIVEACTIVITY_SHOW_SOFT_INPUT_FORCED (1) forces it to show.
    ANativeActivity_showSoftInput(activity, ANATIVEACTIVITY_SHOW_SOFT_INPUT_FORCED);
    keyboard_visible = true;
}

void AndroidInput::hide_soft_keyboard() {
    ANativeActivity* activity = get_native_activity();
    if (!activity) return;
    
    // ANATIVEACTIVITY_HIDE_SOFT_INPUT_IMPLICIT_ONLY (0) hides it safely.
    ANativeActivity_hideSoftInput(activity, ANATIVEACTIVITY_HIDE_SOFT_INPUT_IMPLICIT_ONLY);
    keyboard_visible = false;
}

bool AndroidInput::is_keyboard_visible() {
    return keyboard_visible;
}
