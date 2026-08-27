# IZANAMI
**A fully on-device Android AI companion. She sees, feels, speaks, shows - and soon dances and dissects. No cloud. No telemetry. Yours.**

Native C++17 + raylib on Termux, powered by llama.cpp + mtmd vision, LuaJIT scripting, sqlite memory vault. Runs entirely on a Snapdragon 888 phone.

## FEATURES (SHIPPED)
- **On-device LLM chat** with multi-core hot-swap router (chat / code / vision cores)
- **Real vision (5.5-C)**: attach a photo - she swaps to Qwen3.5-4B, loads mmproj eyes, and describes what she sees
- **Gallery attach** with photo chip, clear button, live keyboard sync
- **LuaJIT VM** for hot-tunable behavior (future: soul.lua personality engine)
- **Memory Vault** (sqlite): ledgers, gotchas, future episodic soul memory
- **Forge / PatchExecutor**: the self-patching pipeline that built itself
- **CrashGuard**, package manager, purple pixel-terminal aesthetic

## ARCHITECTURE
Java MainActivity (picker / mailbox / IME) -> native raylib loop -> ChatEngine -> ModelRouter -> InferenceEngine (llama + mtmd) -> LuaEngine hooks -> MemoryVault persistence.

## MODELS (on-device, /sdcard/Download/Ai offline models/)
Dolphin3.0-Qwen2.5-3B (chat) - dolphin-2.6-mistral-7B - Qwen2.5-Coder-7B (code) - Qwen3.5-4B-Uncensored + mmproj (vision) - Qwen3.5-9B.

## BUILD FROM SOURCE
Termux + Android SDK (ndk/cmake/ninja), then `bash ~/izanami_build.sh` with the app closed. APK lands in Downloads.

## RELEASES & APK DOWNLOADS (newest first)

## ROADMAP (full spec: BLUEPRINT_MVP.md)
7.0 Soul Matrix (emotions / styles / memory / initiative) -> 6.0 Face (chisa avatar, lip sync) -> 6.1 Stage (fullscreen her) -> 6.2 Jukebox + 6.3 Dance -> 6.4 Voice input -> 8.0 Scalpel + Rosetta (on-device RE agent) -> 8.5 Library + Godot presets.

## PROJECT DOCS
- `BLUEPRINT_MVP.md` - complete MVP specification and phase designs
- `HANDOFF.md` - session handoff: state, file map, next steps
- `GOTCHA_LEDGER.md` - every bug, root cause, and fix

Personal research project. Uncensored local models on a user-owned device; all data stays on-device.
- PHASE5_5C (2026-08-27) - Phase 5.5-C: She Sees - [Download APK](https://github.com/arrBisto/izanami-mvp/releases/download/PHASE5_5C/Izanami.apk)
- PHASE5_5B (2026-08-26) - Phase 5.5-B: Gallery Attach + Eyes Button - [Download APK](https://github.com/arrBisto/izanami-mvp/releases/download/PHASE5_5B/Izanami.apk)
- PHASE5_5A (2026-08-25) - Phase 5.5-A: Live Auto-Router - [Download APK](https://github.com/arrBisto/izanami-mvp/releases/download/PHASE5_5A/Izanami.apk)
- PHASE5_5 (2026-08-25) - Phase 5.5: Matrix Splash + Router Defaults - [Download APK](https://github.com/arrBisto/izanami-mvp/releases/download/PHASE5_5/Izanami.apk)
- PHASE5_4B (2026-08-24) - Phase 5.4-B: Micro-Phase Executor - [Download APK](https://github.com/arrBisto/izanami-mvp/releases/download/PHASE5_4B/Izanami.apk)
- PHASE5_4B_AUTOROUTE (2026-08-24) - Phase 5.4-A+: Auto-Router - [Download APK](https://github.com/arrBisto/izanami-mvp/releases/download/PHASE5_4B_AUTOROUTE/Izanami.apk)
- PHASE5_4A (2026-08-24) - Phase 5.4-A: Model Router - [Download APK](https://github.com/arrBisto/izanami-mvp/releases/download/PHASE5_4A/Izanami.apk)
- PHASE5_UI (2026-08-23) - Phase 5: System IME, Haptics, Copy/Save/Attach, Watchdog - [Download APK](https://github.com/arrBisto/izanami-mvp/releases/download/PHASE5_UI/Izanami.apk)
- PHASE4_BASELINE (2026-08-23) - Phase 4: Locked Baseline (Custom Keypad, Pre-System IME) - [Download APK](https://github.com/arrBisto/izanami-mvp/releases/download/PHASE4_BASELINE/Izanami.apk)
