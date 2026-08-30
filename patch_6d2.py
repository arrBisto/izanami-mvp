import os
os.chdir(os.path.expanduser("~/Izanami"))
def patch(path, old, new):
    s = open(path, encoding="utf-8", errors="ignore").read()
    if old in s:
        open(path, "w", encoding="utf-8").write(s.replace(old, new, 1))
        print("ok:", path)
    else:
        print("WARN:", path)

patch("native/Avatar.cpp",
"    if (data->skins_count > 0) {\n        const cgltf_skin* skin = &data->skins[0];",
"    for (size_t s = 0; s < data->skins_count; s++) {\n        const cgltf_skin* skin = &data->skins[s];")
patch("native/Avatar.cpp",
"    } else {\n        for (size_t i = 0; i < nc; i++) out.bones[i].inv_bind = MatrixInvert(globals[i]);",
"    }\n    if (data->skins_count == 0) {\n        for (size_t i = 0; i < nc; i++) out.bones[i].inv_bind = MatrixInvert(globals[i]);")
patch("native/Avatar.cpp",
"                if (jnt) { cgltf_uint jidx[4]={0,0,0,0}; cgltf_accessor_read_uint(jnt->data, v, jidx, 4); for(int k=0;k<4;k++) av.joints[k]=(int)jidx[k]; } else { av.joints[0]=av.joints[1]=av.joints[2]=av.joints[3]=0; }",
"                if (jnt) { cgltf_skin* sk2 = node->skin ? node->skin : (data->skins_count ? &data->skins[0] : nullptr); cgltf_uint jidx[4]={0,0,0,0}; cgltf_accessor_read_uint(jnt->data, v, jidx, 4); for(int k=0;k<4;k++) av.joints[k] = (sk2 && jidx[k] < sk2->joints_count) ? (int)(sk2->joints[jidx[k]] - data->nodes) : 0; } else { av.joints[0]=av.joints[1]=av.joints[2]=av.joints[3]=0; }")
