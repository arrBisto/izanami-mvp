import os
os.chdir(os.path.expanduser("~/Izanami"))
old = '''            cgltf_image* img = gm->pbr_metallic_roughness.base_color_texture.texture->image;
            std::string uri = img->uri ? img->uri : "";
            std::string full = uri.empty() ? "" : (base + uri);
            if (!full.empty() && full.find("data:") == std::string::npos) {
                Image rimg = LoadImage(full.c_str());
                if (rimg.data) {
                    out.mats[i].tex = LoadTextureFromImage(rimg);
                    out.mats[i].has_tex = true;
                    UnloadImage(rimg);
                }
            }'''
new = '''            cgltf_image* img = gm->pbr_metallic_roughness.base_color_texture.texture->image;
            Image rimg = {0};
            std::string uri = img->uri ? img->uri : "";
            if (!uri.empty() && uri.find("data:") == std::string::npos) {
                rimg = LoadImage((base + uri).c_str());
            } else if (img->buffer_view) {
                const cgltf_buffer_view* bv = img->buffer_view;
                const unsigned char* d = (const unsigned char*)bv->buffer->data + bv->offset;
                const char* mt = img->mime_type ? img->mime_type : "image/png";
                const char* ft = (strstr(mt, "jpeg") || strstr(mt, "jpg")) ? "jpg" : "png";
                rimg = LoadImageFromMemory(ft, d, (int)bv->size);
            }
            if (rimg.data) {
                out.mats[i].tex = LoadTextureFromImage(rimg);
                out.mats[i].has_tex = true;
                UnloadImage(rimg);
            }'''
s = open("native/Avatar.cpp", encoding="utf-8", errors="ignore").read()
if old in s:
    open("native/Avatar.cpp", "w", encoding="utf-8").write(s.replace(old, new, 1))
    print("ok: embedded texture support added")
else:
    print("WARN: anchor missing")
