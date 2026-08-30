#pragma once
#include <string>
#include <vector>
#include "raylib.h"
#include "raymath.h"

struct AvatarProbe { int nodes = 0, joints = 0, skins = 0, meshes = 0, materials = 0; };

struct AVertex {
    float pos[3];
    float norm[3];
    float uv[2];
    float weights[4];
    int joints[4];
};

struct AMesh {
    std::string name;
    std::vector<AVertex> verts;
    std::vector<unsigned int> indices;
    int material = 0;
    int node_idx = 0;
    Matrix rest;
    Mesh rmesh;
    bool gpu_ready = false;
};

struct AMaterial {
    std::string name;
    Texture2D tex = {0};
    bool has_tex = false;
    bool unlit = false;
    Color base = {255, 255, 255, 255};
    Material rmat;
};

struct ABone {
    std::string name;
    int parent = -1;
    Matrix local;      // rest local TRS
    Matrix local_u;
    Matrix inv_bind;   // inverse bind (identity when not a joint)
};

struct AvatarModel {
    std::string tag;
    std::vector<AMesh> meshes;
    std::vector<AMaterial> mats;
    std::vector<ABone> bones;   // index == glTF node index
    Matrix root;                // normalization (scale + orient)
    float height = 0.0f;
    int total_verts = 0, total_indices = 0;
    double load_ms = 0.0;
    bool loaded = false;
    std::vector<Matrix> pose_local;
    std::vector<int> topo;
    std::vector<Matrix> skin_m;
    std::vector<int> spring_bones;
    std::vector<Vector3> spring_vel;
    std::vector<Quaternion> spring_rot;
    std::vector<float> spring_amp;
    int b_pelvis = -1, b_spine = -1, b_chest = -1, b_head = -1;
};

class Avatar {
public:
    static AvatarProbe probe(const std::string& path, const std::string& tag);
    static bool load(AvatarModel& out, const std::string& path, const std::string& tag);
    static void upload(AvatarModel& m);
    static void draw(const AvatarModel& m);
    static void rig_idle(AvatarModel& m);
    static void pose_idle(AvatarModel& m, double t);
    static void skin_update(AvatarModel& m);
    static void unload(AvatarModel& m);
};
