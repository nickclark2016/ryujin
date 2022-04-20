#ifndef funcs_glsl_
#define funcs_glsl_

vec3 x_axis_from_quat(vec4 quat)
{
    float fty = 2.0 * quat.y;
    float ftz = 2.0 * quat.z;
    float ftwy = fty * quat.w;
    float ftwz = ftz * quat.w;
    float ftxy = fty * quat.x;
    float ftxz = ftz * quat.x;
    float ftyy = ftz * quat.y;
    float ftzz = ftz * quat.z;

    return vec3(1.0 - (ftyy + ftzz), ftxy + ftwz, ftxz - ftwy);
}

vec3 y_axis_from_quat(vec4 quat)
{
    float ftx = 2.0 * quat.x;
    float fty = 2.0 * quat.y;
    float ftz = 2.0 * quat.z;
    float ftwx = ftx * quat.w;
    float ftwz = ftz * quat.w;
    float ftxx = ftx * quat.x;
    float ftxy = fty * quat.x;
    float ftyz = ftz * quat.y;
    float ftzz = ftz * quat.z;

    return vec3(ftxy - ftwz, 1.0 - (ftxx + ftzz), ftyz + ftwx);
}

void decode_tbn_quaternion(vec4 encodedTbn, out vec3 normal, out vec3 tangent, out vec3 bitangent)
{
    vec4 quat = normalize(encodedTbn);
    normal = x_axis_from_quat(quat);
    tangent = y_axis_from_quat(quat);
    float bitangentReflection = sign(encodedTbn.w);
    bitangent = cross(normal, tangent) * bitangentReflection;
}

#endif // funcs_glsl_