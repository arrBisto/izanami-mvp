# IZANAMI — Native Android LLM Engine (MVP)

Izanami is a military-grade, pure C++17 native Android application that runs Large Language Models (LLMs) entirely offline and on-device. It completely bypasses the standard Android Java JVM, Gradle, and XML layouts, running directly through NativeActivity and rendering via Raylib.

## Core MVP Features

- **Pure C++17 AAA Architecture** — zero-overhead, bare-metal design using std::mutex for strict thread safety between the UI and inference loops.
- **On-Device LLM Inference** — integrates llama.cpp (modern API with llama_vocab and llama_sampler) to run models like Qwen2.5 3B entirely offline.
- **Auto-Router (live core swapping)** — a keyword classifier routes every request to the best core: CODE tasks hot-swap to Qwen2.5-Coder-7B, REASON tasks to Mistral, casual CHAT stays on the fast Dolphin3.0-Qwen2.5-3B base. Swaps happen before generation (race-free), match case-insensitively, and can be toggled with `/autoroute on|off` (persisted).
- **Matrix Splash Screen** — OLED-black boot with a falling katakana waterfall (bundled NotoSansJP-Bold, Unicode range 0x30A0-0x30FF), white-hot heads over three purple depths, a bloomed halo the rain falls around, the app icon filling the halo, pulsing purple name, and tap-to-enter after a 3-second minimum.
- **Thinking-Leak Defense** — hardened system persona forbids printing reasoning; live and end-of-generation stripping of think blocks guarantees clean answers even on thinking GGUFs.
- **Micro-Phase Executor** — `/draft` makes the AI write a bash patch against the app's own source; a y/n Sanity Valve gates it; the Forge watcher applies approved scripts; sanitizer and busy-guard protect the pipeline.
- **Custom Raylib Keypad** — bypasses Android 11+ InputMethodManager NativeActivity restrictions with a custom immediate-mode QWERTY keypad rendered in C++. Multi-line cursor tracking, hold-to-repeat arrows, auto-scroll chat history. System soft-keyboard also supported.
- **AMOLED Neon UI** — perfectly scaling uniform UI based on screen-width fractions. Pure black background with neon purple (180,70,255) accents.
- **Gallery Attach** — Qwen-Studio style round + button inside the chat box opens the native Android gallery (ACTION_GET_CONTENT, image/*); the chosen photo lands as a thumbnail chip with one-tap clear. Vision inference (llava + mmproj auto-pair) lands in Phase 5.5-C.
- **LuaJIT VM** — on-device scripting engine for extensible automation.
- **MemoryVault** — SQLite persistence at /sdcard/Izanami/memory/vault.db, including a gotcha_ledger table where every fixed bug is recorded with error signature, root cause, and fix.
- **CrashGuard** — black-box signal handler, cross-session crash reports, 90s watchdog; a previous crash is auto-injected into chat for AI post-mortem.
- **Model Picker** — scans /sdcard/Izanami/models for GGUF cores; vision companion mmproj files are hidden from the chat picker and reserved for auto-pairing.

## Architecture

    native/
      main.cpp            Raylib UI loop, splash, chat render, + button
      InferenceEngine.*   llama.cpp wrapper, hot-swap, boot priority
      ChatEngine.*        prompt assembly, streaming, router hook, think strip
      ModelRouter.*       CODE/REASON/CHAT keyword classifier
      Attach.*            gallery mailbox polling, thumbnail chip
      AndroidInput.*      JNI bridge (soft keyboard, haptics, image picker)
      java_src/           MainActivity (NativeActivity + intents)
      Forge.*             patch watcher / applier
      PatchExecutor.*     sanitizer + bash runner
      MemoryVault.*       SQLite vault
      CrashGuard.*        signals, watchdog, black box
      LuaEngine.*         LuaJIT VM
      Keypad.*            immediate-mode keypad
      TextWrap.*          word-aware wrapping

## Building From Source (Termux)

1. Install Termux packages: NDK toolchain, cmake, ninja, ImageMagick, zip, and a JDK for javac.
2. Place GGUF models in /sdcard/Izanami/models/.
3. Keep izanami_logo1.png / izanami_logo2.png in Downloads and the NotoSansJP fonts in native/assets (the build script wires them automatically).
4. Run:

    bash ~/izanami_build.sh

The script compiles llama.cpp and the native sources with the NDK, generates launcher icons, zips assets (icon + katakana font) into the APK, signs it, and deploys Izanami.apk to Downloads.

## Release History

- PHASE5_5A — live Auto-Router (race-free swap, expanded keywords)
- PHASE5_5 — Matrix splash, 3B boot default, think strip, mmproj filter, font pipeline
- PHASE5_4B — Micro-Phase Executor (/draft, Sanity Valve, Forge, sanitizer, busy guard)
- PHASE5_4B_AUTOROUTE — first Auto-Router classifier
- PHASE5_UI — AMOLED neon UI + custom keypad

## Roadmap

- 5.5-C VISION — llava wiring + mmproj auto-pair so attached photos are actually seen
- Qwen-style bottom-sheet model picker
- Perf-aware turbo router (RAM/thermal-based core choice)
- IME resume glitch fix (keyboard state after app switch)
