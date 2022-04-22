#ifndef funcs_glsl_
#define funcs_glsl_

vec3 extract_normal(const vec4 q)
{
    return vec3( 0.0,  0.0,  1.0) +
        vec3( 2.0, -2.0, -2.0) * q.x * q.zwx +
        vec3( 2.0,  2.0, -2.0) * q.y * q.wzy;
}

vec3 extract_tangent(const vec4 q)
{
    return vec3( 1.0,  0.0,  0.0) +
        vec3(-2.0,  2.0, -2.0) * q.y * q.yxw +
        vec3(-2.0,  2.0,  2.0) * q.z * q.zwx;
}

void decode_tbn_quaternion(vec4 encodedTbn, out vec3 normal, out vec3 tangent, out vec3 bitangent)
{
    vec4 quat = normalize(encodedTbn);
    normal = normalize(extract_normal(quat));
    tangent = normalize(extract_tangent(quat));
    vec3 binorm = cross(normal, tangent);
    bitangent = sign(encodedTbn.w) * binorm;
}

#endif // funcs_glsl_