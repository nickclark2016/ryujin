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
} fs_in;

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
    uint matId = instances[fs_in.instanceID].material;
    material mat = materials[matId];

    if (mat.albedo >= texturesLoaded)
    {
        fragColor = vec4(1, 1, 1, 1);
    }
    else
    {
        // ambient
        vec4 color = texture(textures[mat.albedo], fs_in.texcoord0);
        vec4 ambient = 0.05 * color;
        // diffuse
        vec3 lightDir = normalize(-lighting.sun.direction);
        vec3 normal = normalize(fs_in.normal);
        float diff = max(dot(lightDir, normal), 0.0);
        vec4 diffuse = diff * color;
        // specular
        vec3 viewDir = normalize(cameras[activeCamera].position - fs_in.worldPosition);
        vec3 reflectDir = reflect(-lightDir, normal);
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        float spec = pow(max(dot(fs_in.normal, halfwayDir), 0.0), 32.0);
        vec3 specular = vec3(0.0196, 0.0157, 0.0157) * spec; // assuming bright white light color
        fragColor = ambient + diffuse + vec4(specular, 0.0);
    }
}