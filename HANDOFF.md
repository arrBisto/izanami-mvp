# HANDOFF - state at PHASE5_5C (2026-08-27)

## WHAT WORKS / TEST
- Chat + router hot-swap. Attach photo via + -> send "what do you see?" -> core flips to Qwen3.5-4B, "Loading eyes...", she describes image (~20s first run).
- Vision log: cat /sdcard/Izanami/memory/vision_log.txt (expect img=, swapped core=, mmproj loaded=1).

## FILE MAP
- native/: main.cpp(raylib loop), ChatEngine, ModelRouter, InferenceEngine(llama+mtmd), Attach, LuaEngine, MemoryVault(sqlite), Forge, PatchExecutor, Keypad, TextWrap, CrashGuard, PackageManager.
- native/java_src/MainActivity.java: gallery pick + downscale 768px + mailbox write.
- CMakeLists: links llama, ggml, mtmd, raylib, luajit, curl, sqlite3; -Wl,--allow-multiple-definition.

## BUILD/INSTALL
- Close app. bash ~/izanami_build.sh. APK -> ~/storage/downloads/Izanami.apk. Install manually.

## GOTCHA TOP HITS
1 Evidence before patching: grep real code + logs first. 2 Anchor/region replace, never whitespace-exact blobs. 3 Prints inside matched branches only. 4 [image:] is display-only; engine self-fetches via last_image.txt. 5 mtmd-helper inside mtmd target. 6 stb dupes need allow-multiple-definition. 7 Full-res vision encode freezes CPU; keep 768px downscale. 8 Qwen3.5 leaks </think> (polish).

## USER PREFERENCES
- Heredoc single-copy blocks; app closed for builds; sanity valve = stop & audit; chat between phases; evidence-first; no loops.

## NEXT SESSION STARTS HERE (7.0 recon)
grep -n "system\|prompt" native/ChatEngine.cpp | head -n 20
grep -rn "kokoro\|tts\|TTS" native/ | head -n 20
grep -n "classify\|MODEL_" native/ModelRouter.cpp | head -n 20
Then build Soul Core + Lua binding per BLUEPRINT_MVP.md 3.9.
