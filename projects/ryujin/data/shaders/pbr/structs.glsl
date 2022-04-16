#ifndef structs_glsl_
#define structs_glsl_

layout (constant_id = 0) const uint MAX_INSTANCE_COUNT = 1024 * 512;
layout (constant_id = 1) const uint MAX_MATERIAL_COUNT = 1024 * 64;
layout (constant_id = 2) const uint MAX_TEXTURE_COUNT = 256;
layout (constant_id = 3) const uint MAX_POINT_LIGHT_COUNT = 512;

struct material
{
    uint albedo;
    uint normal;
    uint metallicRoughness;
    uint emissive;
    uint ambientOcclusion;
};

struct scene_camera
{
    mat4 view;
    mat4 proj;
    mat4 viewProj; // proj * view
    vec3 position;
    vec3 orientation;
};

struct point_light
{
    vec3 position;
    vec3 color;
};

struct scene_lighting
{
    point_light lights[MAX_POINT_LIGHT_COUNT];
    uint pointLightCount;
};

struct instance_data
{
    mat4 transform;
    uint material;
    uint parent;
    uint pad0;
    uint pad1;
};

#endif // structs_glsl_