import os
os.chdir(os.path.expanduser("~/Izanami"))
def patch(path, old, new):
    s = open(path, encoding="utf-8", errors="ignore").read()
    if old in s:
        open(path, "w", encoding="utf-8").write(s.replace(old, new, 1))
        print("ok:", path)
    else:
        print("WARN:", path)

patch("native/Avatar.cpp", "    float maxd = 0;",
"    float maxd = 0; std::string worst;")
patch("native/Avatar.cpp", "        if (!mm.vertices || !mm.normals) continue;",
"        if (!mm.vertices || !mm.normals) continue;\n        float md = 0;")
patch("native/Avatar.cpp",
"            float dx = p.x - mm.vertices[v*3+0];\n            if (fabsf(dx) > maxd) maxd = fabsf(dx);",
"            float dx = p.x - mm.vertices[v*3+0];\n            if (fabsf(dx) > md) md = fabsf(dx);")
patch("native/Avatar.cpp", "        UpdateMeshBuffer(mm, 0,",
"        if (md > maxd) { maxd = md; worst = am.name; }\n        UpdateMeshBuffer(mm, 0,")
patch("native/Avatar.cpp",
'        if (log) log << "skin " << m.tag << " maxd=" << maxd << "\\n";',
'        if (log) {\n            log << "skin " << m.tag << " maxd=" << maxd << " worst=" << worst << "\\n";\n            int bs[4] = {m.b_pelvis, m.b_spine, m.b_chest, m.b_head};\n            for (int q = 0; q < 4; q++) {\n                int bi = bs[q];\n                if (bi >= 0) {\n                    const Matrix& M = m.skin_m[bi];\n                    log << "pm " << bi << " t=(" << M.m12 << "," << M.m13 << "," << M.m14 << ") d=(" << M.m0 << "," << M.m5 << "," << M.m10 << ")\\n";\n                }\n            }\n        }')
