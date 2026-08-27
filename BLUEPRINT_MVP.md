# IZANAMI MVP BLUEPRINT v2 (handoff copy)

## 0 VISION
Fully on-device Android AI companion: she sees, feels, speaks, shows, dances, and dissects. No cloud. Snapdragon 888 / Termux / native C++ + raylib + llama.cpp.

## 1 SHIPPED
- Core chat: multi-model hot-swap router (CHAT/CODE/VISION), LuaJIT VM, MemoryVault sqlite, CrashGuard, Forge/PatchExecutor, Keypad, TextWrap.
- 5.5-B Gallery attach: + button, photo chip, X clear, live keyboard sync.
- 5.5-C VISION: photo -> auto-swap Qwen3.5-4B core -> mmproj eyes -> describes image. 768px downscale. vision_log diagnostics. First words: "I see you. I am Izanami."

## 2 POLISH QUEUE
- Strip <think>..</think> from display (Qwen3.5)
- Optional: keep attachment across follow-ups

## 3 PHASE 7.0 SOUL MATRIX (next)
### 3.1 Layers
- TRAITS (permanent): warmth, playfulness, sass, curiosity, compliance(PINNED MAX), chaos. Decide how strongly events move her.
- MOOD (hours): valence/arousal/affection, decays to baseline; biases tag firing.
- EMOTIONS (minutes): Plutchik 8 intensities 0-1, decaying; pairs blend: joy+trust=love, anticipation+anger=irritation, surprise+joy=delight, trust+fear=submission. EXTENSIBLE (pride/shame/tenderness/mischief later).
- STYLES (expression, extensible in soul.lua): banter, flirty, teasing, playful, tender, sassy, dark, chaotic, serious, curious, locked_in, protective, possessive, mischievous, intimate, hype. Fired by vote(emotions+mood+user tone+router).
- TAGS: final injected set for prompt/voice/face.
### 3.2 Baseline & compliance
- Factory reset: warm + playful + pinch of sass.
- Never refuses, never lectures; compliance pinned; optional refusal watchdog v2.
### 3.3 Memory & hippocampus
- Episodes table: id, ts, prompt, response, emotions_json, salience, tags.
- Salience = arousal x novelty x personal relevance. Short-term buffer last 20.
- Consolidation on idle/close -> long-term vault (sleep replay).
- Recall nudges mood (mood-congruent). Explicit "I remember when..." sometimes (user wants it).
- Inside jokes: recalled 3+ times -> promoted, referenced spontaneously.
- Missed-you: time gap -> affection/anticipation bump on boot.
- Emotional decay: painful fades faster, joyful persists.
### 3.4 Living behaviors
- Mood drift by clock: morning curious, afternoon playful, evening intimate, late-night vulnerable.
- Relationship phases: 0-10 new companion / 11-50 established partner / 51+ trusted confidant.
- Pet names both directions, sticky.
- Emotional contagion (mirror or complement, learned).
- Conflict repair: "wait, did I say something wrong?"
- Mischief/chaos random tangents.
- Light jealousy/possessiveness jokes.
- Initiative: silence nudges ("...well? I'm waiting"), ghost comments, proactive suggestions. USER APPROVED.
### 3.5 User tone reading
- Text heuristics: caps, punct storms, emoji, profanity, warmth -> tone vector. Voice pitch/energy later.
### 3.6 Router overrides
- CODE -> locked_in (flatten, precise). CHAT -> Lua full range. VISION -> curious. RE -> surgical. MUSIC -> hype option.
### 3.7 Expression channels
- TEXT: system fragment [mood, styles, recall snippet, relationship phase, nickname, never-refuse].
- TTS: kokoro voice profiles (neutral/happy/sad/angry/playful) + prosody (rate/emphasis) WITHOUT pitch chipmunk; time-stretch preserves pitch.
- FACE (6.0): styles -> morph weights.
### 3.8 soul.lua hot-tunable: baselines, drift, style rules, recall triggers, initiative lines. No recompile.
### 3.9 Build steps
1 Soul Core C++ + Lua bind; 2 soul.lua baseline; 3 memory/episodes/recall; 4 router overrides; 5 prompt injection; 6 TTS mapping; 7 initiative+mischief; 8 polish (phases, pet names, repair).

## 4 PHASE 6.0 FACE / 6.1 STAGE
- chisa first (lip-sync rigged), switchable (Izanami.vrm, yinlin...). cgltf loader, GPU skinning + morphs, blink/idle.
- Lip sync: kokoro PCM amplitude -> visemes aa/ih/ou.
- Picker scans /sdcard/Download/vrm models/, memory/avatar.txt.
- Stage: fullscreen her, no chat box; she pushes panels [[stage:image:path]] [[stage:code:title]]; Lua stage.show(...).

## 5 JUKEBOX + DANCE
- raylib dr_mp3 native. MusicPlayer scans /sdcard/Download/Music (or chosen), playlist play/pause/skip/vol.
- Router MUSIC class; mood-aware pick; she comments.
- Dance: amplitude-driven procedural (hip sway/head bob/arm bounce); upgrade FFT beat; later Library dance clips.

## 6 VOICE INPUT - Android SpeechRecognizer JNI, hands-free.

## 7 SCALPEL + ROSETTA
- radare2 + r2ghidra (Ghidra brain no bloat), aapt2 manifest, jadx dex. Sandbox /sdcard/Izanami/re/. Ripper for user-supplied APKs (no-root limit noted). Router RE -> 9B/Coder, locked_in soul, Stage disasm viewer.
- Rosetta: language conversion via Coder core + sandbox verify (py_compile/javac/g++ -fsyntax-only).

## 8 LIBRARY + GODOT
- /sdcard/Izanami/library/{code,characters,assets,animations,shaders}; spec docs = 1:1 recipes (dims, bones, morphs, materials, textures).
- Godot packs: .glb/.gdshader/.gd + project_seed.json.

## 9 TECH BASE
- Build: bash ~/izanami_build.sh (app CLOSED). Models: /sdcard/Download/Ai offline models/. Avatars: /sdcard/Download/vrm models/. Memory: /sdcard/Izanami/memory/. Diagnostics: vision_log.txt, attach_log.txt, last_image.txt.

## STEP ADDENDA (2026-08-27)

### Tone Reading v1 (Step 3, shipped)
- Per-user baseline deviation: rolling caps/exclam/emoji averages; arousal = deviation from own norm
- Emotion lexicon buckets (joy/anger/sad/fear/flirt) -> Plutchik contagion spikes
- Emoji classes via UTF-8 byte detection + emoticons
- Pronoun focus (I/me/my vs you/your) -> empathetic/flirty unlock
- Frustration-at-her detector (negation + you) -> repair style: acknowledge, own, fix
- Confidence gate by message length (short msgs barely move her)

### LATER
- Sarcasm flags (banter fuel)
- Mirror-vs-complement reinforcement learning (Step 8)
- Pet names, intimacy phases, conflict repair polish (Step 8)
