import os
os.chdir(os.path.expanduser("~/Izanami"))
def patch(path, old, new):
    s = open(path, encoding="utf-8", errors="ignore").read()
    if old in s:
        open(path, "w", encoding="utf-8").write(s.replace(old, new, 1))
        print("ok:", path)
    else:
        print("WARN:", path)

patch("native/Avatar.hpp", "    std::vector<Quaternion> spring_rot;",
"    std::vector<Quaternion> spring_rot;\n    std::vector<float> spring_amp;")

patch("native/Avatar.cpp",
"                m.spring_bones.push_back((int)i);\n                m.spring_vel.push_back({0, 0, 0});\n                m.spring_rot.push_back(QuaternionIdentity());",
"                m.spring_bones.push_back((int)i);\n                m.spring_vel.push_back({0, 0, 0});\n                m.spring_rot.push_back(QuaternionIdentity());\n                float amp = 0.4f;\n                if (n.find(\"Hair\") != std::string::npos) amp = 0.12f;\n                else if (n.find(\"Bust\") != std::string::npos || n.find(\"Pectoral\") != std::string::npos || n.find(\"pectoral\") != std::string::npos) amp = 0.3f;\n                m.spring_amp.push_back(amp);")

patch("native/Avatar.cpp",
"        float angle = sinf(t_f * 1.3f + i * 0.15f) * 0.45f;",
"        float angle = sinf(t_f * 1.3f + i * 0.15f) * m.spring_amp[k];")
