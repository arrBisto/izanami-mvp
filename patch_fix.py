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
"        UploadMesh(&mm, false);\n        free(mm.vertices); mm.vertices = nullptr;\n        free(mm.normals); mm.normals = nullptr;\n        free(mm.texcoords); mm.texcoords = nullptr;\n        free(mm.indices); mm.indices = nullptr;",
"        UploadMesh(&mm, false);")
patch("native/Avatar.cpp",
"        static Material dbg = LoadMaterialDefault();\n        DrawMesh(am.rmesh, dbg, MatrixMultiply(m.root, am.rest));",
"        DrawMesh(am.rmesh, m.mats[mi].rmat, MatrixMultiply(m.root, am.rest));")
