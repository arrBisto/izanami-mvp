# IZANAMI: SYSTEM KEYBOARD HANDOFF
Status: COMPLETE 2026-08-23. System IME (hidden EditText + TextWatcher + nativeOnText/nativeOnEnter) is the input method; custom purple keypad retired from draw loop but kept in Keypad.cpp as fallback.
Bridge: JNI_OnLoad caches JavaVM + MainActivity.self static field; AndroidBridge::get_jvm/get_activity are the ONLY valid JNI entry points now.
Gotchas: never attach/detach on Java-thread callbacks; GetWindowHandle is null on raylib-android; IME send = IME_ACTION_SEND -> nativeOnEnter -> submit + clearEdit.
Pending: watchdog rewrite onto AndroidBridge; long-press-copy chat messages; + attach button; gap tuned to 0.36f.
