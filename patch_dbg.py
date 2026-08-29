import os
os.chdir(os.path.expanduser("~/Izanami"))
def patch(path, old, new):
    s = open(path, encoding="utf-8", errors="ignore").read()
    if old in s:
        open(path, "w", encoding="utf-8").write(s.replace(old, new, 1))
        print("ok:", path)
    else:
        print("WARN:", path)

patch("native/Avatar.cpp", "        UploadMesh(&mm, false);",
"        unsigned int mx = 0; for (unsigned int i2 : am.indices) if (i2 > mx) mx = i2;\n        if (log) log << \"up \" << am.name << \" vc=\" << mm.vertexCount << \" tc=\" << mm.triangleCount << \" maxidx=\" << mx << \"\\n\";\n        UploadMesh(&mm, false);")
patch("native/Avatar.cpp", "        DrawMesh(am.rmesh, m.mats[mi].rmat, MatrixMultiply(m.root, am.rest));",
"        static Material dbg = LoadMaterialDefault();\n        DrawMesh(am.rmesh, dbg, MatrixMultiply(m.root, am.rest));")
