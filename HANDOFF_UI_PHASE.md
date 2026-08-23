# IZANAMI: UI/UX PHASE HANDOFF
Status: COMPLETE 2026-08-24.
Delivered: system IME input (hidden EditText bridge), IME haptics + haptic2 JNI bridge, 2s hold-to-copy with clipboard + flash, /save AI export to Downloads/Izanami, + attach picker tagging [image: path] into sent messages, watchdog on AndroidBridge (30s leash), input auto-scroll, hard-wrap renderer, split font sizes (header 0.035f / chat 0.05f).
Architecture: AndroidBridge (JNI_OnLoad cached JavaVM + MainActivity.self) is the ONLY JNI entry. JNI callbacks on Java threads must use the passed JNIEnv - never attach/detach.
Vision: attachments tagged now; real sight arrives via llama.cpp mmproj (Qwen2-VL GGUF) in the Model Router phase.
Next: 5.4-A Model Router, 5.4-B Micro-Phase Executor.
