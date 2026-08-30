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
"    addrot(m.b_chest, (Vector3){1, 0, 0}, br * 1.2f);\n    addrot(m.b_spine, (Vector3){0, 0, 1}, sw * 1.5f);\n    addrot(m.b_pelvis, (Vector3){0, 1, 0}, sw2 * 1.0f);\n    addrot(m.b_head, (Vector3){0, 1, 0}, -sw2 * 2.0f + sinf((float)t * 0.47f) * 2.0f);\n    addrot(m.b_head, (Vector3){1, 0, 0}, sinf((float)t * 0.31f) * 1.5f);",
"    addrot(m.b_chest, (Vector3){1, 0, 0}, br * 0.5f);\n    addrot(m.b_spine, (Vector3){0, 0, 1}, sw * 0.4f);\n    addrot(m.b_pelvis, (Vector3){0, 1, 0}, sw2 * 0.25f);\n    addrot(m.b_head, (Vector3){0, 1, 0}, -sw2 * 0.5f + sinf((float)t * 0.47f) * 0.8f);\n    addrot(m.b_head, (Vector3){1, 0, 0}, sinf((float)t * 0.31f) * 0.5f);")
patch("native/Avatar.cpp", "    if (now - last < 1.0 / 30.0) return;", "    if (now - last < 1.0 / 60.0) return;")
