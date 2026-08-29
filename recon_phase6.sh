#!/bin/bash
cd ~/Izanami || exit 1
echo "=== PHASE 6 RECON ==="

echo; echo "--- 1. existing 3d/math/audio libs in project ---"
find . -maxdepth 4 -type f \( -iname '*cgltf*' -o -iname '*glm*' -o -iname '*tinyobj*' -o -iname '*dr_mp3*' -o -iname '*dr_wav*' -o -iname '*marching*' \) ! -path './native/raylib/*' ! -path './submodules/llama.cpp/*' 2>/dev/null
echo "(empty above = none yet)"

echo; echo "--- 2. CMakeLists anchors (sources + links) ---"
grep -nE "add_library|\.cpp|target_link|include_direct|whole-archive|allow-multiple|\.h" native/CMakeLists.txt | head -80

echo; echo "--- 3. raylib version ---"
grep -rh "define RAYLIB_VERSION" native/raylib/src/raylib.h native/raylib/include/raylib.h 2>/dev/null | head -2

echo; echo "--- 4. chisa texture formats ---"
ls "/sdcard/Download/vrm models/chisa gltf/textures/" 2>/dev/null

echo; echo "--- 5. models: nodes/joints/presets/springs ---"
if command -v python3 >/dev/null 2>&1; then
python3 - << 'PY'
import struct, json, os
def load_glb_json(p):
    b = open(p,'rb').read()
    if b[:4] == b'glTF':
        off = 12
        while off + 8 <= len(b):
            clen, ctype = struct.unpack('<I4s', b[off:off+8])
            if ctype == b'JSON':
                return json.loads(b[off+8:off+8+clen].decode('utf-8', errors='ignore'))
            off += 8 + clen
    return json.loads(b.decode('utf-8', errors='ignore'))

base = "/sdcard/Download/vrm models/"
for f in ["chisa gltf/scene.gltf", "chun li gltf/scene.gltf", "Izanami.vrm", "yinlin.vrm", "lena.vrm"]:
    p = os.path.join(base, f)
    if not os.path.exists(p):
        print(f, ": MISSING"); continue
    try:
        d = load_glb_json(p)
        nodes = d.get("nodes", [])
        skins = d.get("skins", [])
        joints = skins[0].get("joints", []) if skins else []
        print("==", f, "| nodes:", len(nodes), "| joints:", len(joints))
        if joints:
            jn = [nodes[i].get("name","?") for i in joints[:40]]
            print("   joints:", ", ".join(jn))
        v = d.get("extensions", {}).get("VRM", {})
        if v:
            gs = v.get("blendShapeMaster", {}).get("blendShapeGroups", [])
            presets = sorted({(g.get("presetName") or g.get("name") or "?") for g in gs})
            sb = v.get("secondaryAnimation", {})
            print("   presets:", presets)
            print("   springGroups:", len(sb.get("boneGroups", [])), "| colliderGroups:", len(sb.get("colliderGroups", [])))
    except Exception as e:
        print(f, ": ERR", e)
PY
else
echo "python3 not found - will fall back to grep"
fi

echo; echo "--- 6. toolchain ---"
which curl python3 clang++ 2>/dev/null

echo; echo "--- 7. music count (jukebox later) ---"
ls "/sdcard/Download/Music/" 2>/dev/null | wc -l
echo "=== END ==="
