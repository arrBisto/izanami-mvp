#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
#include "Avatar.hpp"
#include <fstream>
#include <set>
#include <cstring>
#include <algorithm>

AvatarProbe Avatar::probe(const std::string& path, const std::string& tag) {
    AvatarProbe p;
    std::ofstream log("/sdcard/Izanami/memory/avatar_log.txt", std::ios::app);
    cgltf_options options;
    std::memset(&options, 0, sizeof(options));
    cgltf_data* data = nullptr;
    if (cgltf_parse_file(&options, path.c_str(), &data) != cgltf_result_success || !data) {
        if (log) log << "probe " << tag << " PARSE_FAIL\n";
        return p;
    }
    if (cgltf_load_buffers(&options, data, path.c_str()) != cgltf_result_success) {
        if (log) log << "probe " << tag << " BUFFER_FAIL\n";
        cgltf_free(data);
        return p;
    }
    p.nodes = (int)data->nodes_count;
    p.skins = (int)data->skins_count;
    p.meshes = (int)data->meshes_count;
    p.materials = (int)data->materials_count;
    std::set<std::string> jn;
    for (size_t s = 0; s < data->skins_count; s++)
        for (size_t j = 0; j < data->skins[s].joints_count; j++)
            jn.insert(data->skins[s].joints[j]->name ? data->skins[s].joints[j]->name : "?");
    p.joints = (int)jn.size();
    bool have_bbox = false;
    float bmin[3] = {1e9f, 1e9f, 1e9f}, bmax[3] = {-1e9f, -1e9f, -1e9f};
    for (size_t m = 0; m < data->meshes_count; m++)
        for (size_t pr = 0; pr < data->meshes[m].primitives_count; pr++)
            for (size_t a = 0; a < data->meshes[m].primitives[pr].attributes_count; a++) {
                const cgltf_attribute& at = data->meshes[m].primitives[pr].attributes[a];
                if (at.type == cgltf_attribute_type_position && at.data->has_min && at.data->has_max) {
                    for (int k = 0; k < 3; k++) {
                        bmin[k] = std::min(bmin[k], (float)at.data->min[k]);
                        bmax[k] = std::max(bmax[k], (float)at.data->max[k]);
                    }
                    have_bbox = true;
                }
            }
    if (log) {
        log << "probe " << tag << " nodes=" << p.nodes << " joints=" << p.joints
            << " skins=" << p.skins << " meshes=" << p.meshes << " mats=" << p.materials;
        if (have_bbox)
            log << " bbox=(" << bmin[0] << "," << bmin[1] << "," << bmin[2] << ")-("
                << bmax[0] << "," << bmax[1] << "," << bmax[2] << ")";
        log << "\n";
    }
    cgltf_free(data);
    return p;
}
