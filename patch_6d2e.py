import os
os.chdir(os.path.expanduser("~/Izanami"))
old = '    m.b_head = findb("Head", "head");'
new = '''    m.b_head = findb("Head", "head");
    for (size_t i = 0; i < m.bones.size(); i++) {
        const std::string& n = m.bones[i].name;
        if (n.find("Skirt") != std::string::npos || n.find("Hair") != std::string::npos || n.find("Piao") != std::string::npos || n.find("Bust") != std::string::npos || n.find("Pectoral") != std::string::npos || n.find("pectoral") != std::string::npos) {
            if (m.bones[i].parent >= 0) {
                m.spring_bones.push_back((int)i);
                m.spring_vel.push_back({0, 0, 0});
                m.spring_rot.push_back(QuaternionIdentity());
            }
        }
    }
    {
        std::ofstream lg("/sdcard/Izanami/memory/avatar_log.txt", std::ios::app);
        if (lg) lg << "springs " << m.tag << " n=" << m.spring_bones.size() << "\\n";
    }'''
s = open("native/Avatar.cpp", encoding="utf-8", errors="ignore").read()
if old in s:
    open("native/Avatar.cpp", "w", encoding="utf-8").write(s.replace(old, new, 1))
    print("ok: detection inserted")
else:
    print("WARN: anchor missing")
