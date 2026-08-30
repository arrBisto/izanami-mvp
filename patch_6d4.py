import os
os.chdir(os.path.expanduser("~/Izanami"))
def patch(path, old, new):
    s = open(path, encoding="utf-8", errors="ignore").read()
    if old in s:
        open(path, "w", encoding="utf-8").write(s.replace(old, new, 1))
        print("ok:", path)
    else:
        print("WARN:", path)

patch("native/Avatar.hpp", "    Matrix local;      // rest local TRS",
"    Matrix local;      // rest local TRS\n    Matrix local_u;")
patch("native/Avatar.cpp", "#include <cstdlib>", "#include <cstdlib>\n#include <functional>")
patch("native/Avatar.cpp",
"        out.bones[i].local = mat_from_node(&data->nodes[i]);\n        out.bones[i].inv_bind = MatrixIdentity();\n    }",
"""        out.bones[i].local = mat_from_node(&data->nodes[i]);
        out.bones[i].inv_bind = MatrixIdentity();
    }
    {
        std::vector<Matrix> Gc(nc, MatrixIdentity());
        std::vector<char> vis(nc, 0);
        std::function<void(size_t)> gc = [&](size_t i) {
            if (vis[i]) return;
            int p = out.bones[i].parent;
            Matrix Lu = out.bones[i].local;
            float sx = sqrtf(Lu.m0*Lu.m0 + Lu.m1*Lu.m1 + Lu.m2*Lu.m2);
            float sy = sqrtf(Lu.m4*Lu.m4 + Lu.m5*Lu.m5 + Lu.m6*Lu.m6);
            float sz = sqrtf(Lu.m8*Lu.m8 + Lu.m9*Lu.m9 + Lu.m10*Lu.m10);
            if (sx < 1e-6f) sx = 1; if (sy < 1e-6f) sy = 1; if (sz < 1e-6f) sz = 1;
            Lu.m0 /= sx; Lu.m1 /= sx; Lu.m2 /= sx;
            Lu.m4 /= sy; Lu.m5 /= sy; Lu.m6 /= sy;
            Lu.m8 /= sz; Lu.m9 /= sz; Lu.m10 /= sz;
            out.bones[i].local_u = Lu;
            if (p >= 0) { gc((size_t)p); Gc[i] = MatrixMultiply(Gc[p], Lu); } else Gc[i] = Lu;
            vis[i] = 1;
            out.bones[i].inv_bind = MatrixInvert(Gc[i]);
        };
        for (size_t i = 0; i < nc; i++) gc(i);
    }""")
patch("native/Avatar.cpp",
"    {\n        for (size_t i = 0; i < nc; i++) out.bones[i].inv_bind = MatrixInvert(globals[i]);\n    }",
"")
patch("native/Avatar.cpp",
"    for (size_t i = 0; i < m.bones.size(); i++) m.pose_local[i] = m.bones[i].local;",
"    for (size_t i = 0; i < m.bones.size(); i++) m.pose_local[i] = m.bones[i].local_u;")
