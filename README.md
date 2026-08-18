# IZANAMI: Native Android LLM Engine (MVP)

Izanami is a military-grade, pure C++17 native Android application that runs Large Language Models (LLMs) entirely offline and on-device. It completely bypasses the standard Android Java JVM, Gradle, and XML layouts, running directly through NativeActivity and rendering via Raylib.

## Core MVP Features

* **Pure C++17 AAA Architecture:** Zero-overhead, bare-metal design using std::mutex for strict thread safety between the UI and inference loops.
* **On-Device LLM Inference:** Integrates llama.cpp (modern API with llama_vocab and llama_sampler) to run models like Qwen 2.5 3B entirely offline.
* **Custom Raylib Keypad:** Bypasses Android 11+ InputMethodManager NativeActivity restrictions by rendering a custom, immediate-mode QWERTY keypad directly in C++. Features multi-line cursor tracking, hold-to-repeat arrow keys, and auto-scroll chat history.
* **AMOLED Neon UI:** Perfectly scaling uniform UI based on screen width fractions. Pure black background with neon purple accents.
* **LuaJIT Scripting Engine:** Permanent LuaJIT state engine wired to preprocess user inputs before hitting the LLM.
* **libcurl Package Manager:** Network backend wired to fetch and download Lua tool packages from a GitHub remote registry.

## Development Environment

This project is built 100% on-device using Termux. No external PC, no Android Studio, no Gradle.

* **Toolchain:** clang++ (ARM64), cmake, make, patchelf, aapt2, apksigner, openjdk-17.
* **Target:** minSdkVersion 21, targetSdkVersion 29 (Required for legacy storage access).
* **Architecture:** arm64-v8a

## Failsafe Restoration

To instantly restore the exact working state of this project on any Termux environment, run the failsafe script.
