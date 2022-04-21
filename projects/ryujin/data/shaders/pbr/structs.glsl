#ifndef structs_glsl_
#define structs_glsl_

layout (constant_id = 0) const uint MAX_INSTANCE_COUNT = 1024 * 512;
layout (constant_id = 1) const uint MAX_MATERIAL_COUNT = 1024 * 64;
layout (constant_id = 2) const uint MAX_TEXTURE_COUNT = 256;
layout (constant_id = 3) const uint MAX_POINT_LIGHT_COUNT = 512;
layout (constant_id = 4) const uint MAX_SPOT_LIGHT_COUNT = 512;
layout (constant_id = 5) const uint MAX_CAMERA_COUNT = 32;

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

struct directional_light
{
    vec3 direction;
    float intensity;
};

struct point_light
{
    vec3 position;
    vec3 color;
    vec3 attenuation;
    float range;
};

struct spot_light
{
    vec3 position;
    vec3 rotation;
    vec3 color;
    vec3 attenuation;
    float range;
    float innerRadius;
    float outerRadius;
};

struct ambient_lighting
{
    vec3 color;
    float intensity;
};

struct scene_lighting
{
    uint numPointLights;
    uint numSpotLights;
    uint pad0;
    uint pad1;
    point_light pointLights[MAX_POINT_LIGHT_COUNT];
    spot_light spotLights[MAX_SPOT_LIGHT_COUNT];
    directional_light sun;
    ambient_lighting sceneAmbient;
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