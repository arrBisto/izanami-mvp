# IZANAMI BLUEPRINT (living document)

## SHIPPED
- 5.5-B Gallery attach: + button, photo chip, X clear, live keyboard sync
- 5.5-C VISION: attach photo -> auto-swap Qwen3.5-4B core -> mmproj eyes -> she sees. 768px downscale. vision_log.txt diagnostics.

## POLISH QUEUE
- Strip <think>...</think> from displayed output (Qwen3.5 thinking leak)
- Vision: keep attachment across follow-ups option

## 7.0 SOUL MATRIX (next)
- Mood vector (valence/arousal) + tags (sassy/dark/tender/joking/locked_in), decay to baseline
- Modes keyed to router: CODE=locked_in terse; CHAT=full range; VISION=curious
- User tone heuristics (caps/punct/emoji/profanity); voice pitch later
- Expression: prompt mood fragment injection + kokoro voice/pitch/rate mapping + avatar morphs
- emotion_ledger table; reinforcement of styles that keep user talking
- Lives in soul.lua (LuaJIT) - tune personality without recompiles

## 6.0 FACE
- chisa first (lip-sync rigged), switchable (Izanami.vrm etc)
- cgltf loader, GPU skeletal skinning + morph targets, blink/idle
- Lip sync: kokoro PCM amplitude -> visemes aa/ih/ou
- Avatar picker scanning /sdcard/Download/vrm models/, saved to memory/avatar.txt

## 6.1 STAGE (her screen)
- Fullscreen avatar, no chat box; tap/back returns
- She pushes content panels: [[stage:image:path]] [[stage:code:title]] logs/diffs
- Lua API stage.show(...)

## 6.2 VOICE INPUT - Android SpeechRecognizer JNI, hands-free

## 8.0 SCALPEL (RE agent)
- radare2 + r2ghidra (Ghidra brain, no bloat), aapt2 manifest, jadx dex
- Sandbox /sdcard/Izanami/re/; ripper for user-supplied APKs (no root limit noted)
- Router class RE -> 9B/Coder core; locked_in soul; Stage disasm viewer
- ROSETTA: code language conversion via Coder core + sandbox verify (py_compile/javac/g++ -fsyntax-only)

## 8.5 LIBRARY
- Categories: code/characters/assets/animations/shaders in /sdcard/Izanami/library/
- Character spec docs: dimensions, bones, morphs, materials, textures = 1:1 recipe
- Godot export packs: .glb/.gdshader/.gd + project_seed.json
