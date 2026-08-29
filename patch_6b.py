import os
os.chdir(os.path.expanduser("~/Izanami"))
def patch(path, old, new):
    s = open(path, encoding="utf-8", errors="ignore").read()
    if old in s:
        open(path, "w", encoding="utf-8").write(s.replace(old, new, 1))
        print("ok:", path)
    else:
        print("WARN: anchor missing:", path)

patch("native/Avatar.cpp",
"    out.load_ms = now_ms() - t0;\n    out.loaded = true;",
"    out.load_ms = now_ms() - t0;\n    out.loaded = true;\n    {\n        std::ofstream log(\"/sdcard/Izanami/memory/avatar_log.txt\", std::ios::app);\n        int texn = 0; for (auto& m : out.mats) if (m.has_tex) texn++;\n        if (log) log << \"load \" << tag << \" ok=\" << out.loaded << \" verts=\" << out.total_verts << \" idx=\" << out.total_indices << \" bones=\" << out.bones.size() << \" mats=\" << out.mats.size() << \" tex=\" << texn << \" h=\" << out.height << \" ms=\" << out.load_ms << \"\\n\";\n    }")

patch("native/main.cpp",
"    Avatar::probe(\"/sdcard/Download/vrm models/chisa gltf/scene.gltf\", \"chisa\");\n    Avatar::probe(\"/sdcard/Download/vrm models/chun li gltf/scene.gltf\", \"chunli\");\n    Avatar::probe(\"/sdcard/Download/vrm models/Izanami.vrm\", \"izanami\");",
"    { static AvatarModel av_chisa; Avatar::load(av_chisa, \"/sdcard/Download/vrm models/chisa gltf/scene.gltf\", \"chisa\"); }\n    { static AvatarModel av_iza; Avatar::load(av_iza, \"/sdcard/Download/vrm models/Izanami.vrm\", \"izanami\"); }")
