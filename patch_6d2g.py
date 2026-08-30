import os
os.chdir(os.path.expanduser("~/Izanami"))
def patch(path, old, new):
    s = open(path, encoding="utf-8", errors="ignore").read()
    if old in s:
        open(path, "w", encoding="utf-8").write(s.replace(old, new, 1))
        print("ok:", path)
    else:
        print("WARN:", path)

patch("native/Avatar.cpp",
"    float stiffness = 18.0f;\n    float damping = 6.0f;",
"    float stiffness = 25.0f;\n    float damping = 9.0f;")
patch("native/Avatar.cpp",
"        float angle = sinf(t_f * 1.3f + i * 0.15f) * 1.2f;",
"        float angle = sinf(t_f * 1.3f + i * 0.15f) * 0.45f;")
