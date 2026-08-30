import os
os.chdir(os.path.expanduser("~/Izanami"))
def patch(path, old, new):
    s = open(path, encoding="utf-8", errors="ignore").read()
    if old in s:
        open(path, "w", encoding="utf-8").write(s.replace(old, new, 1))
        print("ok:", path)
    else:
        print("WARN:", path)

patch("native/Avatar.hpp", "    std::vector<Matrix> pose_local;",
"    std::vector<Matrix> pose_local;\n    std::vector<int> topo;")
patch("native/Avatar.cpp",
"    m.b_chest = findb(\"Chest\", \"chest\");\n    m.b_head = findb(\"Head\", \"head\");\n}",
"""    m.b_chest = findb("Chest", "chest");
    if (m.b_chest < 0) m.b_chest = findb("Spine2", nullptr);
    m.b_head = findb("Head", "head");
    {
        std::vector<std::vector<int>> kids(m.bones.size());
        std::vector<int> roots;
        for (size_t i = 0; i < m.bones.size(); i++) {
            int p = m.bones[i].parent;
            if (p >= 0) kids[p].push_back((int)i); else roots.push_back((int)i);
        }
        m.topo.reserve(m.bones.size());
        std::function<void(int)> dfs = [&](int i) {
            m.topo.push_back(i);
            for (int k : kids[i]) dfs(k);
        };
        for (int r : roots) dfs(r);
    }
}""")
patch("native/Avatar.cpp",
"    std::vector<Matrix> G(m.bones.size());\n    for (size_t i = 0; i < m.bones.size(); i++) {\n        int p = m.bones[i].parent;\n        G[i] = (p >= 0) ? MatrixMultiply(G[p], m.pose_local[i]) : m.pose_local[i];\n        m.skin_m[i] = MatrixMultiply(G[i], m.bones[i].inv_bind);\n    }",
"    std::vector<Matrix> G(m.bones.size());\n    for (int i : m.topo) {\n        int p = m.bones[i].parent;\n        G[i] = (p >= 0) ? MatrixMultiply(G[p], m.pose_local[i]) : m.pose_local[i];\n        m.skin_m[i] = MatrixMultiply(G[i], m.bones[i].inv_bind);\n    }")
