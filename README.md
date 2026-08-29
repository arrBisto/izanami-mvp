<p align="center">
  <h1 align="center">IZANAMI</h1>
  <p align="center">
    <strong>A fully on-device Android AI companion. She sees, feels, speaks, shows, dances, and dissects.</strong><br>
    No cloud. No telemetry. No compromises. Yours.
  </p>
  <p align="center">
    <img src="https://img.shields.io/badge/C++-17-blue?style=flat-square&logo=cplusplus" alt="C++17">
    <img src="https://img.shields.io/badge/UI-raylib-purple?style=flat-square" alt="raylib">
    <img src="https://img.shields.io/badge/LLM-llama.cpp-orange?style=flat-square" alt="llama.cpp">
    <img src="https://img.shields.io/badge/Platform-Android-green?style=flat-square&logo=android" alt="Android">
    <img src="https://img.shields.io/badge/Environment-Termux-black?style=flat-square&logo=gnubash" alt="Termux">
  </p>
</p>

## 📖 Overview

**Izanami** is a high-performance, entirely offline AI companion engineered from scratch to run natively on Android hardware (optimized for Snapdragon 888). Built without Java/Gradle bloat, it utilizes a pure **C++17** core with **raylib** for immediate-mode rendering, **llama.cpp** for local LLM inference, and **LuaJIT** for hot-swappable behavioral scripting.

Izanami is a personal research project exploring the absolute limits of mobile edge-computing. All data, memories, and inferences stay strictly on the user-owned device.

## ✨ Features

### 🧠 Core Intelligence (Shipped)
*   **Multi-Core Hot-Swap Router:** Dynamically routes prompts to specialized models (CHAT, CODE, VISION) without dropping the UI thread.
*   **True Local Vision (Phase 5.5-C):** Attach a photo via the native gallery picker. Izanami seamlessly swaps to `Qwen3.5-4B-Uncensored`, loads her `mmproj` vision encoder, downscales the image to 768px for CPU safety, and describes what she sees.
*   **Memory Vault:** SQLite-backed persistent storage for episodic memory, gotchas, and system state.
*   **Forge & PatchExecutor:** A self-aware, self-patching pipeline that built and maintains its own codebase.
*   **CrashGuard & LuaJIT VM:** Hardened C++ wrappers around a permanent Lua state for safe, hot-tunable logic execution.

### 🎭 The Soul Matrix (Active / Shipped Subsystems)
Izanami isn't just a chatbot; she has a state machine simulating personality:
*   **Tone Reading v1:** Analyzes user input for caps, punctuation storms, emoji density, and pronoun focus. Detects frustration and triggers a "conflict repair" conversational style.
*   **Relationship Engine:** Tracks intimacy (0.0 - 1.0) and evolves through phases (Acquaintance → Friend → Confidant → Devoted). Triggers "missed-you" initiatives if left alone for >4 hours.
*   **Mischief Engine:** Tracks "boredom" during dry tasks. Builds "chaos pressure" to trigger spontaneous, playful tangents, and shifts to a delirious, filter-off style between 1 AM and 5 AM.

### 🗺️ Roadmap (Next Up)
*   **Phase 6.0 (Face & Stage):** VRM avatar loading (`cgltf`), GPU skinning, procedural lip-sync via Kokoro TTS visemes, and a fullscreen "Stage" mode.
*   **Phase 6.2/6.3 (Jukebox & Dance):** Native `dr_mp3` music player with FFT beat-detection driving procedural idle animations (hip sway, head bobs).
*   **Phase 7.0 (Deep Soul):** Full Plutchik emotion blending (e.g., joy + trust = love), salience-based memory consolidation, and `soul.lua` hot-tuning.
*   **Phase 8.0 (Scalpel & Rosetta):** An on-device Reverse Engineering agent using `radare2` and `r2ghidra` sandboxed via the Coder LLM.

## 🏗️ Architecture

```text
[Android Java Layer]  -->  Mailbox / IME / Gallery Picker / 768px Downscale
        │
        ▼
[Native C++17 Core]   -->  main.cpp (raylib loop) 
        │                  ├── ChatEngine (Prompt assembly & UI)
        │                  ├── ModelRouter (Auto-classifies CHAT/CODE/VISION)
        │                  ├── InferenceEngine (llama.cpp + mtmd on detached std::thread)
        │                  ├── LuaEngine (LuaJIT hooks for tools/soul)
        │                  └── MemoryVault (SQLite episodic & state persistence)
```

## 📦 Models (On-Device)
Stored locally in `/sdcard/Download/Ai offline models/`.
*   **Chat:** Dolphin3.0-Qwen2.5-3B / dolphin-2.6-mistral-7B
*   **Code:** Qwen2.5-Coder-7B
*   **Vision:** Qwen3.5-4B-Uncensored + `mmproj`
*   **Heavy Lifting:** Qwen3.5-9B

## 🛠️ Building from Source (The Termux Workflow)

Izanami is developed **100% on-device** using Termux. There is no Android Studio, no Gradle, and no desktop cross-compilation. The build system relies on strict Heredoc injections and custom bash scripts.

**Prerequisites:**
```bash
pkg install clang cmake make git gh aapt2 apksigner patchelf zip imagemagick openjdk-17 ndk-sysroot luajit libcurl sqlite
```

**Build Command:**
*Ensure the Izanami app is completely closed before building.*
```bash
bash ~/izanami_build.sh
```
The script handles CMake profiling (`-O3`, `-fPIC`, `-ffast-math`), dependency resolution (stripping versioned Termux `.so` files via `patchelf`), APK zipping, signing, and aligning. The final artifact drops into `~/storage/downloads/Izanami.apk`.

## 📁 Project Structure

```text
~/Izanami/
├── native/                 # C++17 Core
│   ├── main.cpp            # Raylib entry & UI loop
│   ├── ChatEngine.cpp      # UI & Prompt logic
│   ├── ModelRouter.cpp     # LLM routing heuristics
│   ├── InferenceEngine.cpp # llama.cpp/mtmd wrappers
│   ├── LuaEngine.cpp       # LuaJIT VM integration
│   ├── java_src/           # JNI / MainActivity stubs
│   └── raylib/             # raylib submodule
├── submodules/             # llama.cpp, mtmd
├── scripts/                # Lua tooling & soul scripts
├── config/                 # system_profile.json, github_remote.json
└── izanami_build.sh        # Master build & packaging script
```

## 📥 Releases & APK Downloads

| Phase | Release Date | Description | Download |
| :--- | :--- | :--- | :--- |
| **5.5-C** | 2026-08-27 | **She Sees:** Vision mmproj integration. | [Download APK](https://github.com/arrBisto/izanami-mvp/releases/download/PHASE5_5C/Izanami.apk) |
| **5.5-B** | 2026-08-26 | **Gallery Attach:** Eyes button & photo chips. | [Download APK](https://github.com/arrBisto/izanami-mvp/releases/download/PHASE5_5B/Izanami.apk) |
| **5.5-A** | 2026-08-25 | **Live Auto-Router:** Dynamic core swapping. | [Download APK](https://github.com/arrBisto/izanami-mvp/releases/download/PHASE5_5A/Izanami.apk) |
| **5.5** | 2026-08-25 | **Matrix Splash:** Router defaults & UI polish. | [Download APK](https://github.com/arrBisto/izanami-mvp/releases/download/PHASE5_5/Izanami.apk) |
| **5.4-B** | 2026-08-24 | **Micro-Phase Executor:** Self-patching pipeline. | [Download APK](https://github.com/arrBisto/izanami-mvp/releases/download/PHASE5_4B/Izanami.apk) |
| **5.4-A+**| 2026-08-24 | **Auto-Router:** Early routing logic. | [Download APK](https://github.com/arrBisto/izanami-mvp/releases/download/PHASE5_4B_AUTOROUTE/Izanami.apk) |
| **5.4-A** | 2026-08-24 | **Model Router:** Base model swapping. | [Download APK](https://github.com/arrBisto/izanami-mvp/releases/download/PHASE5_4A/Izanami.apk) |

## 📚 Project Documentation

For deep dives into the engineering, state, and battle-tested bug fixes, see the repository docs:
*   [`BLUEPRINT_MVP.md`](./BLUEPRINT_MVP.md) - The complete, immutable MVP specification and phase designs.
*   [`HANDOFF.md`](./HANDOFF.md) - Session handoff: current state, file map, and immediate next steps.
*   [`GOTCHA_LEDGER.md`](./GOTCHA_LEDGER.md) - The master registry of every Termux, C++, raylib, and llama.cpp bug encountered and how it was patched.

---

*Disclaimer: Izanami is a personal research project utilizing uncensored local models on a user-owned device. All data stays on-device. No telemetry is collected.*
