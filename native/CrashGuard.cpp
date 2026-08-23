#include "CrashGuard.hpp"
#include "raylib.h"
#include "AndroidInput.hpp"
#include <android/native_activity.h>
#include <android_native_app_glue.h>
#include <jni.h>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

static const char* CRASH_MARKER   = "/sdcard/Izanami/memory/crash_marker";
static const char* FLIGHT_RECORDER = "/sdcard/Izanami/memory/flight_recorder.log";

static int g_recorder_fd = -1;

static ANativeActivity* get_native_activity() {
    struct android_app* app = static_cast<struct android_app*>(GetWindowHandle());
    if (!app || !app->activity) return nullptr;
    return app->activity;
}

static void append_int(char* buf, int& n, int value) {
    char tmp[12]; int t = 0;
    if (value == 0) tmp[t++] = '0';
    while (value > 0) { tmp[t++] = (char)('0' + value % 10); value /= 10; }
    for (int i = t - 1; i >= 0; --i) buf[n++] = tmp[i];
}

static void crash_signal_handler(int sig) {
    char msg[32]; int n = 0;
    const char* pre = "CRASH_SIGNAL_";
    while (*pre) msg[n++] = *pre++;
    append_int(msg, n, sig);
    msg[n++] = '\n';
    if (g_recorder_fd >= 0) { ssize_t r = write(g_recorder_fd, msg, (size_t)n); (void)r; }
    int fd = open(CRASH_MARKER, O_CREAT | O_WRONLY, 0644);
    if (fd >= 0) { ssize_t r = write(fd, msg, (size_t)n); (void)r; close(fd); }
    signal(sig, SIG_DFL);
    raise(sig);
}

static std::string timestamp_string() {
    char buf[32];
    time_t now = time(nullptr);
    struct tm tm_info;
    localtime_r(&now, &tm_info);
    strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &tm_info);
    return std::string(buf);
}

bool CrashGuard::PreviousSessionCrashed() {
    return access(CRASH_MARKER, F_OK) == 0;
}

std::string CrashGuard::GenerateCrashReport() {
    std::string path = std::string("/sdcard/Izanami/memory/crash_report_") + timestamp_string() + ".md";
    FILE* out = fopen(path.c_str(), "w");
    if (!out) return "";
    fprintf(out, "# IZANAMI INCIDENT REPORT (%s)\n\n## Fatal Signal\n", timestamp_string().c_str());
    FILE* marker = fopen(CRASH_MARKER, "r");
    if (marker) { char b[64]; if (fgets(b, sizeof(b), marker)) fprintf(out, "%s\n", b); fclose(marker); }
    fprintf(out, "\n## Flight Recorder (last 50 events)\n```\n");
    FILE* rec = popen("tail -n 50 /sdcard/Izanami/memory/flight_recorder.log 2>/dev/null", "r");
    if (rec) { char b[256]; while (fgets(b, sizeof(b), rec)) fprintf(out, "%s", b); pclose(rec); }
    fprintf(out, "```\n\n## System Logcat (last 300 lines)\n```\n");
    FILE* log = popen("logcat -d -t 300 2>/dev/null", "r");
    if (log) { char b[512]; while (fgets(b, sizeof(b), log)) fprintf(out, "%s", b); pclose(log); }
    fprintf(out, "```\n");
    fclose(out);
    unlink(CRASH_MARKER);
    return path;
}

void CrashGuard::Initialize() {
    if (g_recorder_fd < 0) {
        g_recorder_fd = open(FLIGHT_RECORDER, O_CREAT | O_WRONLY | O_APPEND, 0644);
    }
    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));
    sa.sa_handler = crash_signal_handler;
    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGABRT, &sa, nullptr);
    sigaction(SIGBUS, &sa, nullptr);
    sigaction(SIGFPE, &sa, nullptr);
    LogEvent("BOOT_OK");
}

void CrashGuard::LogEvent(const std::string& event) {
    if (g_recorder_fd < 0) return;
    std::string line = "[" + timestamp_string() + "] " + event + "\n";
    ssize_t r = write(g_recorder_fd, line.c_str(), line.size());
    (void)r;
}

void CrashGuard::ArmWatchdog(int seconds) {
    JavaVM* vm = AndroidBridge::get_jvm();
    if (!vm) return;
    JNIEnv* env = nullptr;
    if (vm->AttachCurrentThread(&env, nullptr) != JNI_OK) return;
    jobject activity = AndroidBridge::get_activity(env);
    if (!activity) { vm->DetachCurrentThread(); return; }
    jclass activity_class = env->GetObjectClass(activity);
    jmethodID get_service = env->GetMethodID(activity_class, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jstring alarm_str = env->NewStringUTF("alarm");
    jobject alarm_mgr = env->CallObjectMethod(activity, get_service, alarm_str);
    env->DeleteLocalRef(alarm_str);
    if (!alarm_mgr) { vm->DetachCurrentThread(); return; }

    jclass intent_class = env->FindClass("android/content/Intent");
    jmethodID intent_ctor = env->GetMethodID(intent_class, "<init>", "(Landroid/content/Context;Ljava/lang/Class;)V");
    jobject intent = env->NewObject(intent_class, intent_ctor, activity, activity_class);
    jmethodID add_flags = env->GetMethodID(intent_class, "addFlags", "(I)Landroid/content/Intent;");
    env->CallObjectMethod(intent, add_flags, 0x10000000);

    jclass pi_class = env->FindClass("android/app/PendingIntent");
    jmethodID get_activity = env->GetStaticMethodID(pi_class, "getActivity", "(Landroid/content/Context;ILandroid/content/Intent;I)Landroid/app/PendingIntent;");
    jobject pending = env->CallStaticObjectMethod(pi_class, get_activity, activity, 777, intent, (1 << 27) | (1 << 26));

    jclass clock_class = env->FindClass("android/os/SystemClock");
    jmethodID elapsed = env->GetStaticMethodID(clock_class, "elapsedRealtime", "()J");
    jlong now = env->CallStaticLongMethod(clock_class, elapsed);

    jclass mgr_class = env->GetObjectClass(alarm_mgr);
    jmethodID set_method = env->GetMethodID(mgr_class, "set", "(IJLandroid/app/PendingIntent;)V");
    env->CallVoidMethod(alarm_mgr, set_method, 2, now + (jlong)seconds * 1000LL, pending);
    CrashGuard::LogEvent("WATCHDOG: armed");

    env->DeleteLocalRef(pending);
    env->DeleteLocalRef(intent);
    env->DeleteLocalRef(alarm_mgr);
    env->DeleteLocalRef(activity_class);
    vm->DetachCurrentThread();
}

void CrashGuard::DisarmWatchdog() {
    JavaVM* vm = AndroidBridge::get_jvm();
    if (!vm) return;
    JNIEnv* env = nullptr;
    if (vm->AttachCurrentThread(&env, nullptr) != JNI_OK) return;
    jobject activity = AndroidBridge::get_activity(env);
    if (!activity) { vm->DetachCurrentThread(); return; }
    jclass activity_class = env->GetObjectClass(activity);
    jmethodID get_service = env->GetMethodID(activity_class, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
    jstring alarm_str = env->NewStringUTF("alarm");
    jobject alarm_mgr = env->CallObjectMethod(activity, get_service, alarm_str);
    env->DeleteLocalRef(alarm_str);
    if (!alarm_mgr) { vm->DetachCurrentThread(); return; }

    jclass intent_class = env->FindClass("android/content/Intent");
    jmethodID intent_ctor = env->GetMethodID(intent_class, "<init>", "(Landroid/content/Context;Ljava/lang/Class;)V");
    jobject intent = env->NewObject(intent_class, intent_ctor, activity, activity_class);
    jclass pi_class = env->FindClass("android/app/PendingIntent");
    jmethodID get_activity = env->GetStaticMethodID(pi_class, "getActivity", "(Landroid/content/Context;ILandroid/content/Intent;I)Landroid/app/PendingIntent;");
    jobject pending = env->CallStaticObjectMethod(pi_class, get_activity, activity, 777, intent, (1 << 27) | (1 << 26));

    jclass mgr_class = env->GetObjectClass(alarm_mgr);
    jmethodID cancel_method = env->GetMethodID(mgr_class, "cancel", "(Landroid/app/PendingIntent;)V");
    env->CallVoidMethod(alarm_mgr, cancel_method, pending);
    CrashGuard::LogEvent("WATCHDOG: disarmed");

    env->DeleteLocalRef(pending);
    env->DeleteLocalRef(intent);
    env->DeleteLocalRef(alarm_mgr);
    env->DeleteLocalRef(activity_class);
    vm->DetachCurrentThread();
}

void CrashGuard::ShutdownClean() {
    LogEvent("CLEAN_SHUTDOWN");
    DisarmWatchdog();
}
