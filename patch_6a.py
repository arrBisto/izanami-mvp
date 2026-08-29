import os
os.chdir(os.path.expanduser("~/Izanami"))
def patch(path, old, new):
    s = open(path, encoding="utf-8", errors="ignore").read()
    if old in s:
        open(path, "w", encoding="utf-8").write(s.replace(old, new, 1))
        print("ok:", path, "|", old.strip().splitlines()[0][:50])
    else:
        print("WARN: anchor missing in", path, "|", old.strip().splitlines()[0][:50])
patch("native/main.cpp", '#include "Forge.hpp"', '#include "Forge.hpp"\n#include "Avatar.hpp"')
patch("native/main.cpp", '    CrashGuard::Initialize();',
      '    CrashGuard::Initialize();\n'
      '    Avatar::probe("/sdcard/Download/vrm models/chisa gltf/scene.gltf", "chisa");\n'
      '    Avatar::probe("/sdcard/Download/vrm models/chun li gltf/scene.gltf", "chunli");\n'
      '    Avatar::probe("/sdcard/Download/vrm models/Izanami.vrm", "izanami");')
patch("native/CMakeLists.txt", '    PatchExecutor.cpp', '    PatchExecutor.cpp\n    Avatar.cpp')
