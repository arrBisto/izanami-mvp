#include "AndroidInput.hpp"
#include "CrashGuard.hpp"
#include <android/native_activity.h>
#include <android_native_app_glue.h>

static JavaVM* g_jvm = nullptr;
static jclass g_main_class = nullptr;
static jfieldID g_self_field = nullptr;

extern "C" JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    g_jvm = vm;
    JNIEnv* env = nullptr;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) == JNI_OK && env) {
        jclass cls = env->FindClass("com/izanami/MainActivity");
        if (cls) {
            g_main_class = (jclass)env->NewGlobalRef(cls);
            g_self_field = env->GetStaticFieldID(cls, "self", "Landroid/app/Activity;");
        }
        if (env->ExceptionCheck()) env->ExceptionClear();
    }
    return JNI_VERSION_1_6;
}

JavaVM* AndroidBridge::get_jvm() { return g_jvm; }

jobject AndroidBridge::get_activity(JNIEnv* env) {
    if (!g_main_class || !g_self_field) return nullptr;
    jobject act = env->GetStaticObjectField(g_main_class, g_self_field);
    if (env->ExceptionCheck()) { env->ExceptionClear(); return nullptr; }
    return act;
}

void AndroidInput::haptic2(int milliseconds) {
    if (!g_jvm) { CrashGuard::LogEvent("HAPTIC2: no jvm"); return; }
    JNIEnv* env = nullptr;
    if (g_jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) { CrashGuard::LogEvent("HAPTIC2: attach failed"); return; }
    jobject activity = AndroidBridge::get_activity(env);
    if (!activity) { CrashGuard::LogEvent("HAPTIC2: activity null"); g_jvm->DetachCurrentThread(); return; }
    jclass ctx_class = env->GetObjectClass(activity);
    jmethodID get_service = env->GetMethodID(ctx_class, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jstring vib_str = env->NewStringUTF("vibrator");
    jobject vibrator = env->CallObjectMethod(activity, get_service, vib_str);
    env->DeleteLocalRef(vib_str);
    bool done = false;
    if (vibrator) {
        jclass vib_class = env->GetObjectClass(vibrator);
        jclass ve_class = env->FindClass("android/os/VibrationEffect");
        if (ve_class) {
            jmethodID create = env->GetStaticMethodID(ve_class, "createOneShot", "(JI)Landroid/os/VibrationEffect;");
            if (create) {
                jobject effect = env->CallStaticObjectMethod(ve_class, create, (jlong)milliseconds, (jint)255);
                if (env->ExceptionCheck()) env->ExceptionClear();
                else if (effect) {
                    jmethodID v2 = env->GetMethodID(vib_class, "vibrate", "(Landroid/os/VibrationEffect;)V");
                    if (v2) { env->CallVoidMethod(vibrator, v2, effect); done = !env->ExceptionCheck(); if (env->ExceptionCheck()) env->ExceptionClear(); }
                }
            }
        }
        if (!done) {
            jmethodID v1 = env->GetMethodID(vib_class, "vibrate", "(J)V");
            if (v1) { env->CallVoidMethod(vibrator, v1, (jlong)milliseconds); done = !env->ExceptionCheck(); if (env->ExceptionCheck()) env->ExceptionClear(); }
        }
        env->DeleteLocalRef(vib_class);
        env->DeleteLocalRef(vibrator);
    }
    static bool logged = false;
    if (done && !logged) { CrashGuard::LogEvent("HAPTIC2: fired OK"); logged = true; }
    if (!done) CrashGuard::LogEvent("HAPTIC2: failed");
    env->DeleteLocalRef(ctx_class);
    env->DeleteLocalRef(activity);
    g_jvm->DetachCurrentThread();
}

void AndroidInput::show_soft_keyboard() {
    if (!g_jvm) return;
    JNIEnv* env = nullptr;
    if (g_jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) return;
    jobject activity = AndroidBridge::get_activity(env);
    if (!activity) { g_jvm->DetachCurrentThread(); return; }
    jclass act_class = env->GetObjectClass(activity);
    jmethodID show_kb = env->GetMethodID(act_class, "showKeyboard", "()V");
    if (show_kb) env->CallVoidMethod(activity, show_kb);
    env->ExceptionClear();
    env->DeleteLocalRef(act_class);
    env->DeleteLocalRef(activity);
    g_jvm->DetachCurrentThread();
}

void AndroidInput::hide_soft_keyboard() {
    if (!g_jvm) return;
    JNIEnv* env = nullptr;
    if (g_jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) return;
    jobject activity = AndroidBridge::get_activity(env);
    if (!activity) { g_jvm->DetachCurrentThread(); return; }
    jclass act_class = env->GetObjectClass(activity);
    jmethodID hide_kb = env->GetMethodID(act_class, "hideKeyboard", "()V");
    if (hide_kb) env->CallVoidMethod(activity, hide_kb);
    env->ExceptionClear();
    env->DeleteLocalRef(act_class);
    env->DeleteLocalRef(activity);
    g_jvm->DetachCurrentThread();
}

bool AndroidInput::is_keyboard_visible() {
    return false;
}

void AndroidInput::copy_to_clipboard(const std::string& text) {
    if (!g_jvm) return;
    JNIEnv* env = nullptr;
    if (g_jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) return;
    jobject activity = AndroidBridge::get_activity(env);
    if (!activity) { g_jvm->DetachCurrentThread(); return; }
    jclass act_class = env->GetObjectClass(activity);
    jmethodID get_service = env->GetMethodID(act_class, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jstring clip_str = env->NewStringUTF("clipboard");
    jobject manager = env->CallObjectMethod(activity, get_service, clip_str);
    env->DeleteLocalRef(clip_str);
    if (manager) {
        jclass clip_class = env->FindClass("android/content/ClipData");
        jmethodID new_plain = env->GetStaticMethodID(clip_class, "newPlainText", "(Ljava/lang/CharSequence;Ljava/lang/CharSequence;)Landroid/content/ClipData;");
        jstring label = env->NewStringUTF("Izanami");
        jstring content = env->NewStringUTF(text.c_str());
        jobject clip = env->CallStaticObjectMethod(clip_class, new_plain, label, content);
        if (clip) {
            jclass mgr_class = env->GetObjectClass(manager);
            jmethodID set_primary = env->GetMethodID(mgr_class, "setPrimaryClip", "(Landroid/content/ClipData;)V");
            if (set_primary) env->CallVoidMethod(manager, set_primary, clip);
            env->ExceptionClear();
            env->DeleteLocalRef(mgr_class);
            env->DeleteLocalRef(clip);
        }
        env->DeleteLocalRef(clip_class);
        env->DeleteLocalRef(manager);
    }
    env->DeleteLocalRef(act_class);
    env->DeleteLocalRef(activity);
    g_jvm->DetachCurrentThread();
}

std::string AndroidInput::get_clipboard_text() {
    std::string result = "";
    if (!g_jvm) return result;
    JNIEnv* env = nullptr;
    if (g_jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) return result;
    jobject activity = AndroidBridge::get_activity(env);
    if (!activity) { g_jvm->DetachCurrentThread(); return result; }
    jclass act_class = env->GetObjectClass(activity);
    jmethodID get_service = env->GetMethodID(act_class, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jstring clip_str = env->NewStringUTF("clipboard");
    jobject manager = env->CallObjectMethod(activity, get_service, clip_str);
    env->DeleteLocalRef(clip_str);
    if (manager) {
        jclass mgr_class = env->GetObjectClass(manager);
        jmethodID get_primary = env->GetMethodID(mgr_class, "getPrimaryClip", "()Landroid/content/ClipData;");
        jobject clip = env->CallObjectMethod(manager, get_primary);
        env->ExceptionClear();
        if (clip) {
            jclass clip_class = env->GetObjectClass(clip);
            jmethodID get_item = env->GetMethodID(clip_class, "getItemAt", "(I)Landroid/content/ClipData$Item;");
            jobject item = env->CallObjectMethod(clip, get_item, 0);
            env->ExceptionClear();
            if (item) {
                jclass item_class = env->GetObjectClass(item);
                jmethodID get_text = env->GetMethodID(item_class, "getText", "()Ljava/lang/CharSequence;");
                jobject text_obj = env->CallObjectMethod(item, get_text);
                env->ExceptionClear();
                if (text_obj) {
                    jclass cs_class = env->GetObjectClass(text_obj);
                    jmethodID to_string = env->GetMethodID(cs_class, "toString", "()Ljava/lang/String;");
                    jstring str = (jstring)env->CallObjectMethod(text_obj, to_string);
                    if (str) {
                        const char* utf = env->GetStringUTFChars(str, nullptr);
                        if (utf) { result = utf; env->ReleaseStringUTFChars(str, utf); }
                    }
                    env->DeleteLocalRef(cs_class);
                }
                env->DeleteLocalRef(item_class);
                env->DeleteLocalRef(item);
            }
            env->DeleteLocalRef(clip_class);
            env->DeleteLocalRef(clip);
        }
        env->DeleteLocalRef(mgr_class);
        env->DeleteLocalRef(manager);
    }
    env->DeleteLocalRef(act_class);
    env->DeleteLocalRef(activity);
    g_jvm->DetachCurrentThread();
    return result;
}

#include "ChatEngine.hpp"
#include "Attach.hpp"

extern "C" JNIEXPORT void JNICALL Java_com_izanami_MainActivity_nativeOnText(JNIEnv* env, jobject thiz, jstring text) {
    const char* utf = env->GetStringUTFChars(text, nullptr);
    if (utf) {
        ChatEngine::get_instance().set_input(utf);
        env->ReleaseStringUTFChars(text, utf);
    }
}

extern "C" JNIEXPORT void JNICALL Java_com_izanami_MainActivity_nativeOnEnter(JNIEnv* env, jobject thiz) {
    if (Attach::has_attachment()) { ChatEngine::get_instance().append_string("\n[image: " + Attach::attachment_path() + "]"); Attach::clear_attachment(); }
    ChatEngine::get_instance().submit();
    jobject activity = AndroidBridge::get_activity(env);
    if (activity) {
        jclass act_class = env->GetObjectClass(activity);
        jmethodID clear_method = env->GetMethodID(act_class, "clearEdit", "()V");
        if (clear_method) env->CallVoidMethod(activity, clear_method);
        env->ExceptionClear();
        env->DeleteLocalRef(act_class);
        env->DeleteLocalRef(activity);
    }
}
