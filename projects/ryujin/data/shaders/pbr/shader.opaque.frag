#version 460

#include "structs.glsl"

layout (location = 0) out vec4 fragColor;

layout (location = 0) in VS_OUT
{
    vec3 worldPosition;
    vec2 texcoord0;
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    flat int instanceID;
} vs_in;

layout (set = 0, binding = 0) uniform Camera
{
    scene_camera cameras[MAX_CAMERA_COUNT];
};

layout (set = 0, binding = 1) buffer SceneData 
{
    scene_lighting lighting;
    uint texturesLoaded;
};

layout (std430, set = 1, binding = 0) buffer Instances
{
    instance_data instances[MAX_INSTANCE_COUNT];
};

layout (std430, set = 1, binding = 1) buffer Materials
{
    material materials[MAX_MATERIAL_COUNT];
};

layout (set = 1, binding = 2) uniform sampler2D textures[MAX_TEXTURE_COUNT];

layout (push_constant) uniform constants
{
    uint activeCamera;
};

void main(void)
{
    uint matId = instances[vs_in.instanceID].material;
    material mat = materials[matId];

    if (mat.albedo >= texturesLoaded)
    {
        fragColor = vec4(1, 1, 1, 1);
    }
    else
    {
        scene_camera cam = cameras[activeCamera];
        vec4 albedo = texture(textures[mat.albedo], vs_in.texcoord0);
        vec3 toLightDir = -normalize(lighting.sun.direction);
        vec3 toViewDir = normalize(cam.position - vs_in.worldPosition);
        vec3 halfway = normalize(toLightDir + toViewDir);
        float spec = pow(max(dot(vs_in.normal, halfway), 0.0), 16.0);
        vec3 specular = vec3(0.3) * spec;
        fragColor = vec4(lighting.sceneAmbient.color + albedo.xyz + specular, 1.0);
    }
}