import os
os.chdir(os.path.expanduser("~/Izanami"))
old = "    for (size_t n = 0; n < nc; n++) {\n        cgltf_node* node = &data->nodes[n];\n        if (!node->mesh) continue;"
new = """    {
        std::ofstream lg("/sdcard/Izanami/memory/avatar_log.txt", std::ios::app);
        if (lg && data->skins_count) {
            const cgltf_skin* sk = &data->skins[0];
            for (size_t j = 0; j < 3 && j < sk->joints_count; j++) {
                size_t ni = sk->joints[j] - data->nodes;
                Matrix P = MatrixMultiply(globals[ni], out.bones[ni].inv_bind);
                lg << "ibm j" << j << " ni=" << ni << " P_t=(" << P.m12 << "," << P.m13 << "," << P.m14 << ") P_diag=(" << P.m0 << "," << P.m5 << "," << P.m10 << ")\\n";
            }
        }
    }
    for (size_t n = 0; n < nc; n++) {
        cgltf_node* node = &data->nodes[n];
        if (!node->mesh) continue;"""
s = open("native/Avatar.cpp", encoding="utf-8", errors="ignore").read()
if old in s:
    open("native/Avatar.cpp", "w", encoding="utf-8").write(s.replace(old, new, 1))
    print("ok: ibm probe inserted")
else:
    print("WARN: anchor missing")
