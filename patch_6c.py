import os
os.chdir(os.path.expanduser("~/Izanami"))
def patch(path, old, new):
    s = open(path, encoding="utf-8", errors="ignore").read()
    if old in s:
        open(path, "w", encoding="utf-8").write(s.replace(old, new, 1))
        print("ok:", path, "|", old.strip().splitlines()[0][:40])
    else:
        print("WARN:", path, "|", old.strip().splitlines()[0][:40])

patch("native/Avatar.hpp", "    int node_idx = 0;\n};",
"    int node_idx = 0;\n    Matrix rest;\n    Mesh rmesh;\n    bool gpu_ready = false;\n};")
patch("native/Avatar.hpp", "    Color base = {255, 255, 255, 255};\n};",
"    Color base = {255, 255, 255, 255};\n    Material rmat;\n};")
patch("native/Avatar.hpp", "    static bool load(AvatarModel& out, const std::string& path, const std::string& tag);\n    static void unload(AvatarModel& m);",
"    static bool load(AvatarModel& out, const std::string& path, const std::string& tag);\n    static void upload(AvatarModel& m);\n    static void draw(const AvatarModel& m);\n    static void unload(AvatarModel& m);")
patch("native/Avatar.cpp", "#include <cstring>", "#include <cstring>\n#include <cstdlib>")
patch("native/Avatar.cpp", "            am.node_idx = (int)n;",
"            am.node_idx = (int)n;\n            am.rest = globals[am.node_idx];")
patch("native/Avatar.cpp", "void Avatar::unload(AvatarModel& m) {",
'''void Avatar::upload(AvatarModel& m) {
    std::ofstream log("/sdcard/Izanami/memory/avatar_log.txt", std::ios::app);
    int gpu = 0;
    for (auto& am : m.meshes) {
        if ((int)am.verts.size() > 65535 || am.verts.empty() || am.indices.empty()) continue;
        Mesh mm; std::memset(&mm, 0, sizeof(mm));
        mm.vertexCount = (int)am.verts.size();
        mm.triangleCount = (int)(am.indices.size() / 3);
        mm.vertices = (float*)malloc(mm.vertexCount * 3 * sizeof(float));
        mm.normals = (float*)malloc(mm.vertexCount * 3 * sizeof(float));
        mm.texcoords = (float*)malloc(mm.vertexCount * 2 * sizeof(float));
        mm.indices = (unsigned short*)malloc(mm.triangleCount * 3 * sizeof(unsigned short));
        for (int v = 0; v < mm.vertexCount; v++) {
            const AVertex& av = am.verts[v];
            mm.vertices[v*3+0] = av.pos[0]; mm.vertices[v*3+1] = av.pos[1]; mm.vertices[v*3+2] = av.pos[2];
            mm.normals[v*3+0] = av.norm[0]; mm.normals[v*3+1] = av.norm[1]; mm.normals[v*3+2] = av.norm[2];
            mm.texcoords[v*2+0] = av.uv[0]; mm.texcoords[v*2+1] = av.uv[1];
        }
        for (int i = 0; i < mm.triangleCount * 3; i++) mm.indices[i] = (unsigned short)am.indices[i];
        UploadMesh(&mm, false);
        free(mm.vertices); mm.vertices = nullptr;
        free(mm.normals); mm.normals = nullptr;
        free(mm.texcoords); mm.texcoords = nullptr;
        free(mm.indices); mm.indices = nullptr;
        am.rmesh = mm;
        am.gpu_ready = true;
        gpu++;
        am.verts.clear(); am.verts.shrink_to_fit();
        am.indices.clear(); am.indices.shrink_to_fit();
    }
    for (auto& mt : m.mats) {
        mt.rmat = LoadMaterialDefault();
        if (mt.has_tex) mt.rmat.maps[MATERIAL_MAP_ALBEDO].texture = mt.tex;
    }
    if (log) log << "upload " << m.tag << " gpu=" << gpu << "/" << m.meshes.size() << "\\n";
}

void Avatar::draw(const AvatarModel& m) {
    for (const auto& am : m.meshes) {
        if (!am.gpu_ready) continue;
        int mi = am.material;
        if (mi < 0 || mi >= (int)m.mats.size()) mi = 0;
        DrawMesh(am.rmesh, m.mats[mi].rmat, MatrixMultiply(m.root, am.rest));
    }
}

void Avatar::unload(AvatarModel& m) {''')
patch("native/main.cpp", '#include "Avatar.hpp"',
'#include "Avatar.hpp"\n\nstatic AvatarModel g_stage_avatar;\nstatic bool stage_on = false;')
patch("native/main.cpp",
'    { static AvatarModel av_chisa; Avatar::load(av_chisa, "/sdcard/Download/vrm models/chisa gltf/scene.gltf", "chisa"); }\n    { static AvatarModel av_iza; Avatar::load(av_iza, "/sdcard/Download/vrm models/Izanami.vrm", "izanami"); }',
'    Avatar::load(g_stage_avatar, "/sdcard/Download/vrm models/chisa gltf/scene.gltf", "chisa");\n    Avatar::upload(g_stage_avatar);')
patch("native/main.cpp",
'            if (!forge_result.empty()) ChatEngine::get_instance().add_message(ChatRole::ASSISTANT, forge_result);\n        }',
'            if (!forge_result.empty()) ChatEngine::get_instance().add_message(ChatRole::ASSISTANT, forge_result);\n        }\n        if (stage_on) {\n            static Vector2 sdown = {-1, -1};\n            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) sdown = GetMousePosition();\n            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {\n                if (sdown.x >= 0) {\n                    Vector2 t = GetMousePosition();\n                    float dx = t.x - sdown.x, dy = t.y - sdown.y;\n                    if (dx > 90 && fabsf(dy) < 60) stage_on = false;\n                }\n                sdown = {-1, -1};\n            }\n            BeginDrawing();\n            ClearBackground(BLACK);\n            Camera3D cam = {{0.0f, 1.15f, 2.4f}, {0.0f, 0.9f, 0.0f}, {0.0f, 1.0f, 0.0f}, 50.0f, CAMERA_PERSPECTIVE};\n            BeginMode3D(cam);\n            Avatar::draw(g_stage_avatar);\n            EndMode3D();\n            DrawText("< swipe right to return", 12, 12, 20, (Color){180, 70, 255, 255, 255});\n            EndDrawing();\n            continue;\n        }')
patch("native/main.cpp",
'        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {\n            bool tapped_input = CheckCollisionPointRec(GetMousePosition(), input_box);',
'        {\n            static Vector2 cdown = {-1, -1};\n            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) cdown = GetMousePosition();\n            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {\n                if (cdown.x >= 0) {\n                    Vector2 t = GetMousePosition();\n                    float dx = t.x - cdown.x, dy = t.y - cdown.y;\n                    if (dx < -90 && fabsf(dy) < 60) stage_on = true;\n                }\n                cdown = {-1, -1};\n            }\n        }\n        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {\n            bool tapped_input = CheckCollisionPointRec(GetMousePosition(), input_box);')
