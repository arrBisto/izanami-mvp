import os
os.chdir(os.path.expanduser("~/Izanami"))
def patch(path, old, new):
    s = open(path, encoding="utf-8", errors="ignore").read()
    if old in s:
        open(path, "w", encoding="utf-8").write(s.replace(old, new, 1))
        print("ok:", path)
    else:
        print("WARN:", path)

patch("native/Avatar.hpp",
"    std::vector<int> spring_bones;\n    std::vector<Vector3> spring_vel;\n    std::vector<int> spring_bones;\n    std::vector<Vector3> spring_vel;",
"    std::vector<int> spring_bones;\n    std::vector<Vector3> spring_vel;\n    std::vector<Quaternion> spring_rot;")

patch("native/Avatar.cpp",
'        if (n.find("Skirt") != std::string::npos || n.find("Hair") != std::string::npos || n.find("Piao") != std::string::npos) {',
'        if (n.find("Skirt") != std::string::npos || n.find("Hair") != std::string::npos || n.find("Piao") != std::string::npos || n.find("Bust") != std::string::npos || n.find("Pectoral") != std::string::npos || n.find("pectoral") != std::string::npos) {')

patch("native/Avatar.cpp", "                m.spring_vel.push_back({0, 0, 0});",
"                m.spring_vel.push_back({0, 0, 0});\n                m.spring_rot.push_back(QuaternionIdentity());")

old_loop = '''        Matrix R_curr = m.pose_local[i];
        Matrix R_rest = m.bones[i].local_u;
        Quaternion q_curr = QuaternionFromMatrix(R_curr);
        Quaternion q_rest = QuaternionFromMatrix(R_rest);

        Vector3 axis = { sinf(t_f * 1.5f + i * 0.2f), 0, cosf(t_f * 1.1f + i * 0.2f) };
        float len = sqrtf(axis.x*axis.x + axis.z*axis.z);
        if (len > 0.01f) { axis.x /= len; axis.z /= len; } else { axis.x = 1.0f; axis.z = 0.0f; }
        float angle = sinf(t_f * 2.0f + i * 0.3f) * 6.0f;
        Quaternion q_target = QuaternionMultiply(q_rest, QuaternionFromAxisAngle(axis, angle * DEG2RAD));

        Quaternion q_diff = QuaternionMultiply(q_target, QuaternionInvert(q_curr));
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
            q_curr = QuaternionMultiply(q_step, q_curr);
            q_curr = QuaternionNormalize(q_curr);
        }
        m.pose_local[i] = QuaternionToMatrix(q_curr);'''
new_loop = '''        Vector3 axis = { sinf(t_f * 1.5f + i * 0.2f), 0, cosf(t_f * 1.1f + i * 0.2f) };
        float len = sqrtf(axis.x*axis.x + axis.z*axis.z);
        if (len > 0.01f) { axis.x /= len; axis.z /= len; } else { axis.x = 1.0f; axis.z = 0.0f; }
        float angle = sinf(t_f * 2.0f + i * 0.3f) * 6.0f;
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
        m.pose_local[i] = MatrixMultiply(m.bones[i].local_u, QuaternionToMatrix(q_rel));'''
patch("native/Avatar.cpp", old_loop, new_loop)
