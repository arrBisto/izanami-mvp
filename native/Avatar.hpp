#pragma once
#include <string>
struct AvatarProbe { int nodes = 0, joints = 0, skins = 0, meshes = 0, materials = 0; };
class Avatar {
public:
    static AvatarProbe probe(const std::string& path, const std::string& tag);
};
