#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
#include "Avatar.hpp"
#include <fstream>
#include <set>
#include <cstring>
#include <cstdlib>
#include <functional>
#include <algorithm>
#include <cmath>
#include <sys/time.h>

static double now_ms() {
    struct timeval tv; gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

static Matrix mat_from_node(const cgltf_node* n) {
    if (n->has_matrix) {
        const float* m = n->matrix;
        return { m[0],m[4],m[8],m[12], m[1],m[5],m[9],m[13], m[2],m[6],m[10],m[14], m[3],m[7],m[11],m[15] };
    }
    Vector3 t = { n->has_translation ? n->translation[0] : 0, n->has_translation ? n->translation[1] : 0, n->has_translation ? n->translation[2] : 0 };
    Quaternion q = { n->has_rotation ? n->rotation[0] : 0, n->has_rotation ? n->rotation[1] : 0, n->has_rotation ? n->rotation[2] : 0, n->has_rotation ? n->rotation[3] : 1 };
    Vector3 s = { n->has_scale ? n->scale[0] : 1, n->has_scale ? n->scale[1] : 1, n->has_scale ? n->scale[2] : 1 };
    Matrix mT = MatrixTranslate(t.x, t.y, t.z);
    Matrix mR = QuaternionToMatrix(q);
    Matrix mS = MatrixScale(s.x, s.y, s.z);
    return MatrixMultiply(mT, MatrixMultiply(mR, mS));
}

static void compute_global(cgltf_data* d, size_t i, std::vector<Matrix>& G, std::vector<bool>& V) {
    if (V[i]) return;
    cgltf_node* n = &d->nodes[i];
    Matrix local = mat_from_node(n);
    if (n->parent) {
        size_t pi = n->parent - d->nodes;
        compute_global(d, pi, G, V);
        G[i] = MatrixMultiply(G[pi], local);
    } else {
        G[i] = local;
    }
    V[i] = true;
}

AvatarProbe Avatar::probe(const std::string& path, const std::string& tag) {
    AvatarProbe p;
    std::ofstream log("/sdcard/Izanami/memory/avatar_log.txt", std::ios::app);
    cgltf_options options; std::memset(&options, 0, sizeof(options));
    cgltf_data* data = nullptr;
    if (cgltf_parse_file(&options, path.c_str(), &data) != cgltf_result_success || !data) {
        if (log) log << "probe " << tag << " PARSE_FAIL\n"; return p;
    }
    std::string base = path.substr(0, path.find_last_of("/\\") + 1);
    if (cgltf_load_buffers(&options, data, base.c_str()) != cgltf_result_success) {
        if (log) log << "probe " << tag << " BUFFER_FAIL\n"; cgltf_free(data); return p;
    }
    p.nodes = data->nodes_count; p.skins = data->skins_count; p.meshes = data->meshes_count; p.materials = data->materials_count;
    std::set<std::string> jn;
    for (size_t s = 0; s < data->skins_count; s++)
        for (size_t j = 0; j < data->skins[s].joints_count; j++)
            jn.insert(data->skins[s].joints[j]->name ? data->skins[s].joints[j]->name : "?");
    p.joints = (int)jn.size();
    if (log) log << "probe " << tag << " nodes=" << p.nodes << " joints=" << p.joints << " skins=" << p.skins << " meshes=" << p.meshes << " mats=" << p.materials << "\n";
    cgltf_free(data);
    return p;
}

bool Avatar::load(AvatarModel& out, const std::string& path, const std::string& tag) {
    double t0 = now_ms();
    out.tag = tag; out.loaded = false;
    cgltf_options opts = { cgltf_file_type_invalid };
    cgltf_data* data = nullptr;
    if (cgltf_parse_file(&opts, path.c_str(), &data) != cgltf_result_success || !data) return false;
    std::string base = path.substr(0, path.find_last_of("/\\") + 1);
    if (cgltf_load_buffers(&opts, data, base.c_str()) != cgltf_result_success) { cgltf_free(data); return false; }

    size_t nc = data->nodes_count;
    out.bones.resize(nc);
    std::vector<Matrix> globals(nc, MatrixIdentity());
    std::vector<bool> visited(nc, false);
    for (size_t i = 0; i < nc; i++) compute_global(data, i, globals, visited);

    for (size_t i = 0; i < nc; i++) {
        out.bones[i].name = data->nodes[i].name ? data->nodes[i].name : "";
        out.bones[i].parent = data->nodes[i].parent ? (int)(data->nodes[i].parent - data->nodes) : -1;
        out.bones[i].local = mat_from_node(&data->nodes[i]);
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
    }

    for (size_t s = 0; s < 0; s++) {
        const cgltf_skin* skin = &data->skins[s];
        const float* ibm = nullptr;
        if (skin->inverse_bind_matrices && skin->inverse_bind_matrices->buffer_view) {
            cgltf_buffer_view* bv = skin->inverse_bind_matrices->buffer_view;
            ibm = (const float*)((const char*)bv->buffer->data + bv->offset + skin->inverse_bind_matrices->offset);
        }
        for (size_t j = 0; j < skin->joints_count; j++) {
            size_t ni = skin->joints[j] - data->nodes;
            if (ibm) {
                const float* m = ibm + j * 16;
                out.bones[ni].inv_bind = { m[0],m[4],m[8],m[12], m[1],m[5],m[9],m[13], m[2],m[6],m[10],m[14], m[3],m[7],m[11],m[15] };
            } else {
                out.bones[ni].inv_bind = MatrixInvert(globals[ni]);
            }
        }
    }


    {
        std::ofstream lg("/sdcard/Izanami/memory/avatar_log.txt", std::ios::app);
        if (lg && data->skins_count) {
            const cgltf_skin* sk = &data->skins[0];
            for (size_t j = 0; j < 3 && j < sk->joints_count; j++) {
                size_t ni = sk->joints[j] - data->nodes;
                Matrix P = MatrixMultiply(globals[ni], out.bones[ni].inv_bind);
                lg << "ibm j" << j << " ni=" << ni << " P_t=(" << P.m12 << "," << P.m13 << "," << P.m14 << ") P_diag=(" << P.m0 << "," << P.m5 << "," << P.m10 << ")\n";
            }
        }
    }
    for (size_t n = 0; n < nc; n++) {
        cgltf_node* node = &data->nodes[n];
        if (!node->mesh) continue;
        cgltf_mesh* gm = node->mesh;
        for (size_t p = 0; p < gm->primitives_count; p++) {
            cgltf_primitive* pr = &gm->primitives[p];
            AMesh am;
            am.name = gm->name ? gm->name : "";
            am.material = pr->material ? (int)(pr->material - data->materials) : 0;
            am.node_idx = (int)n;
            am.rest = globals[am.node_idx];
            cgltf_attribute *pos=0, *norm=0, *uv=0, *jnt=0, *wgt=0;
            for (size_t a = 0; a < pr->attributes_count; a++) {
                if (pr->attributes[a].type == cgltf_attribute_type_position) pos = &pr->attributes[a];
                else if (pr->attributes[a].type == cgltf_attribute_type_normal) norm = &pr->attributes[a];
                else if (pr->attributes[a].type == cgltf_attribute_type_texcoord && pr->attributes[a].index == 0) uv = &pr->attributes[a];
                else if (pr->attributes[a].type == cgltf_attribute_type_joints && pr->attributes[a].index == 0) jnt = &pr->attributes[a];
                else if (pr->attributes[a].type == cgltf_attribute_type_weights && pr->attributes[a].index == 0) wgt = &pr->attributes[a];
            }
            if (!pos) continue;
            size_t vc = pos->data->count;
            am.verts.resize(vc);
            for (size_t v = 0; v < vc; v++) {
                AVertex& av = am.verts[v];
                cgltf_accessor_read_float(pos->data, v, av.pos, 3);
                if (norm) cgltf_accessor_read_float(norm->data, v, av.norm, 3); else { av.norm[0]=0; av.norm[1]=1; av.norm[2]=0; }
                if (uv) cgltf_accessor_read_float(uv->data, v, av.uv, 2); else { av.uv[0]=0; av.uv[1]=0; }
                if (jnt) { cgltf_skin* sk2 = node->skin ? node->skin : (data->skins_count ? &data->skins[0] : nullptr); cgltf_uint jidx[4]={0,0,0,0}; cgltf_accessor_read_uint(jnt->data, v, jidx, 4); for(int k=0;k<4;k++) av.joints[k] = (sk2 && jidx[k] < sk2->joints_count) ? (int)(sk2->joints[jidx[k]] - data->nodes) : 0; } else { av.joints[0]=av.joints[1]=av.joints[2]=av.joints[3]=0; }
                if (wgt) cgltf_accessor_read_float(wgt->data, v, av.weights, 4); else { av.weights[0]=1; av.weights[1]=av.weights[2]=av.weights[3]=0; }
            }
            if (pr->indices) {
                am.indices.resize(pr->indices->count);
                for (size_t i = 0; i < pr->indices->count; i++) am.indices[i] = (unsigned int)cgltf_accessor_read_index(pr->indices, i);
            } else {
                am.indices.resize(vc); for(size_t i=0; i<vc; i++) am.indices[i] = (unsigned int)i;
            }
            out.meshes.push_back(std::move(am));
        }
    }

    out.mats.resize(data->materials_count);
    for (size_t i = 0; i < data->materials_count; i++) {
        cgltf_material* gm = &data->materials[i];
        out.mats[i].name = gm->name ? gm->name : "";
        out.mats[i].unlit = gm->unlit;
        if (gm->has_pbr_metallic_roughness && gm->pbr_metallic_roughness.base_color_texture.texture && gm->pbr_metallic_roughness.base_color_texture.texture->image) {
            cgltf_image* img = gm->pbr_metallic_roughness.base_color_texture.texture->image;
            Image rimg = {0};
            std::string uri = img->uri ? img->uri : "";
            if (!uri.empty() && uri.find("data:") == std::string::npos) {
                rimg = LoadImage((base + uri).c_str());
            } else if (img->buffer_view) {
                const cgltf_buffer_view* bv = img->buffer_view;
                const unsigned char* d = (const unsigned char*)bv->buffer->data + bv->offset;
                const char* mt = img->mime_type ? img->mime_type : "image/png";
                const char* ft = (strstr(mt, "jpeg") || strstr(mt, "jpg")) ? ".jpg" : ".png";
                rimg = LoadImageFromMemory(ft, d, (int)bv->size);
            }
            if (rimg.data) {
                out.mats[i].tex = LoadTextureFromImage(rimg);
                out.mats[i].has_tex = true;
                UnloadImage(rimg);
            }
        }
    }

    Matrix orient = MatrixIdentity();
    float z_span = 0, y_span = 0;
    for (auto& am : out.meshes) {
        Matrix G = globals[am.node_idx];
        for (auto& v : am.verts) {
            Vector4 pv = { v.pos[0], v.pos[1], v.pos[2], 1.0f };
            Vector4 gv = { G.m0*pv.x + G.m4*pv.y + G.m8*pv.z + G.m12, G.m1*pv.x + G.m5*pv.y + G.m9*pv.z + G.m13, G.m2*pv.x + G.m6*pv.y + G.m10*pv.z + G.m14, 1.0f };
            if (gv.y < -1e8f || gv.y > 1e8f) continue;
            y_span = std::max(y_span, std::abs(gv.y));
            z_span = std::max(z_span, std::abs(gv.z));
        }
    }
    if (z_span > y_span * 1.5f) orient = MatrixRotateX(-PI/2.0f);

    float r_min_x=1e9f, r_max_x=-1e9f, r_min_y=1e9f, r_max_y=-1e9f, r_min_z=1e9f, r_max_z=-1e9f;
    for (auto& am : out.meshes) {
        Matrix G_final = MatrixMultiply(orient, globals[am.node_idx]);
        for (auto& v : am.verts) {
            Vector4 pv = { v.pos[0], v.pos[1], v.pos[2], 1.0f };
            Vector4 gv = { G_final.m0*pv.x + G_final.m4*pv.y + G_final.m8*pv.z + G_final.m12, G_final.m1*pv.x + G_final.m5*pv.y + G_final.m9*pv.z + G_final.m13, G_final.m2*pv.x + G_final.m6*pv.y + G_final.m10*pv.z + G_final.m14, 1.0f };
            if (gv.x < r_min_x) r_min_x = gv.x; if (gv.x > r_max_x) r_max_x = gv.x;
            if (gv.y < r_min_y) r_min_y = gv.y; if (gv.y > r_max_y) r_max_y = gv.y;
            if (gv.z < r_min_z) r_min_z = gv.z; if (gv.z > r_max_z) r_max_z = gv.z;
        }
    }

    float cx = (r_min_x + r_max_x) * 0.5f;
    float cy = (r_min_y + r_max_y) * 0.5f;
    float cz = (r_min_z + r_max_z) * 0.5f;
    out.height = r_max_y - r_min_y;
    float scale = 1.6f / (out.height > 0.01f ? out.height : 1.0f);

    out.root = MatrixMultiply(MatrixTranslate(-cx, -cy, -cz), MatrixMultiply(MatrixScale(scale, scale, scale), orient));

    out.total_verts = 0; out.total_indices = 0;
    for (auto& m : out.meshes) { out.total_verts += m.verts.size(); out.total_indices += m.indices.size(); }

    rig_idle(out);
    out.load_ms = now_ms() - t0;
    out.loaded = true;
    {
        std::ofstream log("/sdcard/Izanami/memory/avatar_log.txt", std::ios::app);
        int texn = 0; for (auto& m : out.mats) if (m.has_tex) texn++;
        if (log) log << "load " << tag << " ok=" << out.loaded << " verts=" << out.total_verts << " idx=" << out.total_indices << " bones=" << out.bones.size() << " mats=" << out.mats.size() << " tex=" << texn << " h=" << out.height << " ms=" << out.load_ms << "\n";
    }
    cgltf_free(data);
    return true;
}

void Avatar::upload(AvatarModel& m) {
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
        unsigned int mx = 0; for (unsigned int i2 : am.indices) if (i2 > mx) mx = i2;
        if (log) log << "up " << am.name << " vc=" << mm.vertexCount << " tc=" << mm.triangleCount << " maxidx=" << mx << "\n";
        UploadMesh(&mm, false);
        am.rmesh = mm;
        am.gpu_ready = true;
        gpu++;
        am.indices.clear(); am.indices.shrink_to_fit();
    }
    for (auto& mt : m.mats) {
        mt.rmat = LoadMaterialDefault();
        if (mt.has_tex) mt.rmat.maps[MATERIAL_MAP_ALBEDO].texture = mt.tex;
    }
    if (log) log << "upload " << m.tag << " gpu=" << gpu << "/" << m.meshes.size() << "\n";
}

void Avatar::draw(const AvatarModel& m) {
    for (const auto& am : m.meshes) {
        if (!am.gpu_ready) continue;
        int mi = am.material;
        if (mi < 0 || mi >= (int)m.mats.size()) mi = 0;
        DrawMesh(am.rmesh, m.mats[mi].rmat, MatrixMultiply(m.root, am.rest));
    }
}

void Avatar::rig_idle(AvatarModel& m) {
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
    if (m.b_chest < 0) m.b_chest = findb("Spine2", nullptr);
    m.b_head = findb("Head", "head");
    for (size_t i = 0; i < m.bones.size(); i++) {
        const std::string& n = m.bones[i].name;
        if (n.find("Skirt") != std::string::npos || n.find("Hair") != std::string::npos || n.find("Piao") != std::string::npos || n.find("Bust") != std::string::npos || n.find("Pectoral") != std::string::npos || n.find("pectoral") != std::string::npos) {
            if (m.bones[i].parent >= 0) {
                m.spring_bones.push_back((int)i);
                m.spring_vel.push_back({0, 0, 0});
                m.spring_rot.push_back(QuaternionIdentity());
                float amp = 0.4f;
                if (n.find("Hair") != std::string::npos) amp = 0.12f;
                else if (n.find("Bust") != std::string::npos || n.find("Pectoral") != std::string::npos || n.find("pectoral") != std::string::npos) amp = 0.3f;
                m.spring_amp.push_back(amp);
            }
        }
    }
    {
        std::ofstream lg("/sdcard/Izanami/memory/avatar_log.txt", std::ios::app);
        if (lg) lg << "springs " << m.tag << " n=" << m.spring_bones.size() << "\n";
    }
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
}

void Avatar::pose_idle(AvatarModel& m, double t) {
    for (size_t i = 0; i < m.bones.size(); i++) m.pose_local[i] = m.bones[i].local_u;
    float br = sinf((float)t * 1.9f);
    float sw = sinf((float)t * 0.6f);
    float sw2 = sinf((float)t * 0.23f + 1.7f);
    auto addrot = [&](int bi, Vector3 axis, float deg) {
        if (bi < 0) return;
        Quaternion q = QuaternionFromAxisAngle(axis, deg * DEG2RAD);
        m.pose_local[bi] = MatrixMultiply(m.pose_local[bi], QuaternionToMatrix(q));
    };
    addrot(m.b_chest, (Vector3){1, 0, 0}, br * 0.5f);
    addrot(m.b_spine, (Vector3){0, 0, 1}, sw * 0.4f);
    addrot(m.b_pelvis, (Vector3){0, 1, 0}, sw2 * 0.25f);
    
    float dt = 0.016f;
    float stiffness = 25.0f;
    float damping = 9.0f;
    float t_f = (float)t;
    for (size_t k = 0; k < m.spring_bones.size(); k++) {
        int i = m.spring_bones[k];
        Vector3 axis = { sinf(t_f * 0.9f + i * 0.1f), 0, cosf(t_f * 0.7f + i * 0.1f) };
        float len = sqrtf(axis.x*axis.x + axis.z*axis.z);
        if (len > 0.01f) { axis.x /= len; axis.z /= len; } else { axis.x = 1.0f; axis.z = 0.0f; }
        float angle = sinf(t_f * 1.3f + i * 0.15f) * m.spring_amp[k];
        Quaternion q_target = QuaternionFromAxisAngle(axis, angle * DEG2RAD);

        Quaternion q_rel = m.spring_rot[k];
        Quaternion q_diff = QuaternionMultiply(q_target, QuaternionInvert(q_rel));
        if (q_diff.w < 0) { q_diff.x = -q_diff.x; q_diff.y = -q_diff.y; q_diff.z = -q_diff.z; q_diff.w = -q_diff.w; }
        Vector3 force = { q_diff.x * 2.0f * stiffness, q_diff.y * 2.0f * stiffness, q_diff.z * 2.0f * stiffness };
        Vector3 damp = { m.spring_vel[k].x * damping, m.spring_vel[k].y * damping, m.spring_vel[k].z * damping };
        Vector3 accel = { force.x - damp.x, force.y - damp.y, force.z - damp.z };

        m.spring_vel[k].x += accel.x * dt;
        m.spring_vel[k].y += accel.y * dt;
        m.spring_vel[k].z += accel.z * dt;

        Vector3 step = { m.spring_vel[k].x * dt, m.spring_vel[k].y * dt, m.spring_vel[k].z * dt };
        float step_len = sqrtf(step.x*step.x + step.y*step.y + step.z*step.z);
        if (step_len > 0.0001f) {
            Quaternion q_step = QuaternionFromAxisAngle({step.x/step_len, step.y/step_len, step.z/step_len}, step_len);
            q_rel = QuaternionNormalize(QuaternionMultiply(q_step, q_rel));
        }
        m.spring_rot[k] = q_rel;
        m.pose_local[i] = MatrixMultiply(m.bones[i].local_u, QuaternionToMatrix(q_rel));
    }
}

void Avatar::skin_update(AvatarModel& m) {
    static double last = 0;
    double now = GetTime();
    if (now - last < 1.0 / 60.0) return;
    last = now;
    std::vector<Matrix> G(m.bones.size());
    for (int i : m.topo) {
        int p = m.bones[i].parent;
        G[i] = (p >= 0) ? MatrixMultiply(G[p], m.pose_local[i]) : m.pose_local[i];
        m.skin_m[i] = MatrixMultiply(G[i], m.bones[i].inv_bind);
    }
    float maxd = 0; std::string worst;
    for (auto& am : m.meshes) {
        if (!am.gpu_ready || am.verts.empty()) continue;
        Mesh& mm = am.rmesh;
        if (!mm.vertices || !mm.normals) continue;
        float md = 0;
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
            if (fabsf(dx) > md) md = fabsf(dx);
            mm.vertices[v*3+0] = p.x; mm.vertices[v*3+1] = p.y; mm.vertices[v*3+2] = p.z;
            mm.normals[v*3+0] = n.x; mm.normals[v*3+1] = n.y; mm.normals[v*3+2] = n.z;
        }
        if (md > maxd) { maxd = md; worst = am.name; }
        UpdateMeshBuffer(mm, 0, mm.vertices, mm.vertexCount * 3 * (int)sizeof(float), 0);
        UpdateMeshBuffer(mm, 3, mm.normals, mm.vertexCount * 3 * (int)sizeof(float), 0);
    }
    static double lastlog = 0;
    if (now - lastlog > 2.0) {
        lastlog = now;
        std::ofstream log("/sdcard/Izanami/memory/avatar_log.txt", std::ios::app);
        if (log) {
            log << "skin " << m.tag << " maxd=" << maxd << " worst=" << worst << "\n";
            int bs[4] = {m.b_pelvis, m.b_spine, m.b_chest, m.b_head};
            for (int q = 0; q < 4; q++) {
                int bi = bs[q];
                if (bi >= 0) {
                    const Matrix& M = m.skin_m[bi];
                    log << "pm " << bi << " t=(" << M.m12 << "," << M.m13 << "," << M.m14 << ") d=(" << M.m0 << "," << M.m5 << "," << M.m10 << ")\n";
                }
            }
        }
    }
}

void Avatar::unload(AvatarModel& m) {
    for (auto& mat : m.mats) if (mat.has_tex) UnloadTexture(mat.tex);
    m.meshes.clear(); m.mats.clear(); m.bones.clear(); m.loaded = false;
}
