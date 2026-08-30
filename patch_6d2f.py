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
"    float stiffness = 12.0f;\n    float damping = 3.0f;",
"    float stiffness = 18.0f;\n    float damping = 6.0f;")
patch("native/Avatar.cpp",
"        Vector3 axis = { sinf(t_f * 1.5f + i * 0.2f), 0, cosf(t_f * 1.1f + i * 0.2f) };",
"        Vector3 axis = { sinf(t_f * 0.9f + i * 0.1f), 0, cosf(t_f * 0.7f + i * 0.1f) };")
patch("native/Avatar.cpp",
"        float angle = sinf(t_f * 2.0f + i * 0.3f) * 6.0f;",
"        float angle = sinf(t_f * 1.3f + i * 0.15f) * 1.2f;")
