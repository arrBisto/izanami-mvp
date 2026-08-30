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
"    for (size_t s = 0; s < data->skins_count; s++) {\n        const cgltf_skin* skin = &data->skins[s];",
"    for (size_t s = 0; s < 0; s++) {\n        const cgltf_skin* skin = &data->skins[s];")
patch("native/Avatar.cpp",
"    if (data->skins_count == 0) {",
"    {")
