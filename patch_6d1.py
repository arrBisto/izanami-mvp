import os
os.chdir(os.path.expanduser("~/Izanami"))
def patch(path, old, new):
    s = open(path, encoding="utf-8", errors="ignore").read()
    if old in s:
        open(path, "w", encoding="utf-8").write(s.replace(old, new, 1))
        print("ok:", path, "|", old.strip().splitlines()[0][:40])
    else:
        print("WARN:", path, "|", old.strip().splitlines()[0][:40])

patch("native/Avatar.hpp", "    double load_ms = 0.0;\n    bool loaded = false;",
"    double load_ms = 0.0;\n    bool loaded = false;\n    std::vector<Matrix> pose_local;\n    std::vector<Matrix> skin_m;\n    int b_pelvis = -1, b_spine = -1, b_chest = -1, b_head = -1;")
patch("native/Avatar.hpp", "    static void draw(const AvatarModel& m);",
"    static void draw(const AvatarModel& m);\n    static void rig_idle(AvatarModel& m);\n    static void pose_idle(AvatarModel& m, double t);\n    static void skin_update(AvatarModel& m);")
patch("native/Avatar.cpp", "    out.load_ms = now_ms() - t0;",
"    rig_idle(out);\n    out.load_ms = now_ms() - t0;")
patch("native/Avatar.cpp", "        am.verts.clear(); am.verts.shrink_to_fit();\n        am.indices.clear(); am.indices.shrink_to_fit();",
"        am.indices.clear(); am.indices.shrink_to_fit();")
patch("native/Avatar.cpp", "void Avatar::unload(AvatarModel& m) {",
'''void Avatar::rig_idle(AvatarModel& m) {
    m.pose_local.resize(m.bones.size());
    m.skin_m.assign(m.bones.size(), MatrixIdentity());
    auto findb = [&](const char* a, const char* b) {
        for (size_t i = 0; i < m.bones.size(); i++) {
            const std::string& n = m.bones[i].name;
            if (n.find(a) != std::string::npos || (b && n.find(b) != std::string::npos)) return (int)i;
        }
        return -1;
    };
    m.b_pelvis = findb("Pelvis", "hip");
    m.b_spine = findb("Spine", "spine");
    m.b_chest = findb("Chest", "chest");
    m.b_head = findb("Head", "head");
}

void Avatar::pose_idle(AvatarModel& m, double t) {
    for (size_t i = 0; i < m.bones.size(); i++) m.pose_local[i] = m.bones[i].local;
    float br = sinf((float)t * 1.9f);
    float sw = sinf((float)t * 0.6f);
    float sw2 = sinf((float)t * 0.23f + 1.7f);
    auto addrot = [&](int bi, Vector3 axis, float deg) {
        if (bi < 0) return;
        Quaternion q = QuaternionFromAxisAngle(axis, deg * DEG2RAD);
        m.pose_local[bi] = MatrixMultiply(m.pose_local[bi], QuaternionToMatrix(q));
    };
    addrot(m.b_chest, (Vector3){1, 0, 0}, br * 1.2f);
    addrot(m.b_spine, (Vector3){0, 0, 1}, sw * 1.5f);
    addrot(m.b_pelvis, (Vector3){0, 1, 0}, sw2 * 1.0f);
    addrot(m.b_head, (Vector3){0, 1, 0}, -sw2 * 2.0f + sinf((float)t * 0.47f) * 2.0f);
    addrot(m.b_head, (Vector3){1, 0, 0}, sinf((float)t * 0.31f) * 1.5f);
}

void Avatar::skin_update(AvatarModel& m) {
    static double last = 0;
    double now = GetTime();
    if (now - last < 1.0 / 30.0) return;
    last = now;
    std::vector<Matrix> G(m.bones.size());
    for (size_t i = 0; i < m.bones.size(); i++) {
        int p = m.bones[i].parent;
        G[i] = (p >= 0) ? MatrixMultiply(G[p], m.pose_local[i]) : m.pose_local[i];
        m.skin_m[i] = MatrixMultiply(G[i], m.bones[i].inv_bind);
    }
    float maxd = 0;
    for (auto& am : m.meshes) {
        if (!am.gpu_ready || am.verts.empty()) continue;
        Mesh& mm = am.rmesh;
        if (!mm.vertices || !mm.normals) continue;
        for (size_t v = 0; v < am.verts.size(); v++) {
            const AVertex& av = am.verts[v];
            Vector3 p = {0, 0, 0}, n = {0, 0, 0};
            for (int k = 0; k < 4; k++) {
                float w = av.weights[k];
                if (w <= 0.0001f) continue;
                const Matrix& M = m.skin_m[av.joints[k]];
                Vector3 vp = {av.pos[0], av.pos[1], av.pos[2]};
                p.x += w * (M.m0*vp.x + M.m4*vp.y + M.m8*vp.z + M.m12);
                p.y += w * (M.m1*vp.x + M.m5*vp.y + M.m9*vp.z + M.m13);
                p.z += w * (M.m2*vp.x + M.m6*vp.y + M.m10*vp.z + M.m14);
                Vector3 vn = {av.norm[0], av.norm[1], av.norm[2]};
                n.x += w * (M.m0*vn.x + M.m4*vn.y + M.m8*vn.z);
                n.y += w * (M.m1*vn.x + M.m5*vn.y + M.m9*vn.z);
                n.z += w * (M.m2*vn.x + M.m6*vn.y + M.m10*vn.z);
            }
            float dx = p.x - mm.vertices[v*3+0];
            if (fabsf(dx) > maxd) maxd = fabsf(dx);
            mm.vertices[v*3+0] = p.x; mm.vertices[v*3+1] = p.y; mm.vertices[v*3+2] = p.z;
            mm.normals[v*3+0] = n.x; mm.normals[v*3+1] = n.y; mm.normals[v*3+2] = n.z;
        }
        UpdateMeshBuffer(mm, 0, mm.vertices, mm.vertexCount * 3 * (int)sizeof(float), 0);
        UpdateMeshBuffer(mm, 3, mm.normals, mm.vertexCount * 3 * (int)sizeof(float), 0);
    }
    static double lastlog = 0;
    if (now - lastlog > 2.0) {
        lastlog = now;
        std::ofstream log("/sdcard/Izanami/memory/avatar_log.txt", std::ios::app);
        if (log) log << "skin " << m.tag << " maxd=" << maxd << "\\n";
    }
}

void Avatar::unload(AvatarModel& m) {''')
patch("native/main.cpp", "            BeginMode3D(cam);\n            Avatar::draw(g_stage_avatar);",
"            BeginMode3D(cam);\n            Avatar::pose_idle(g_stage_avatar, GetTime());\n            Avatar::skin_update(g_stage_avatar);\n            Avatar::draw(g_stage_avatar);")
