#version 460

#include "structs.glsl"

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texcoord0;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec4 tangent;

layout (location = 0) out VS_OUT
{
    vec3 worldPosition;
    vec2 texcoord0;
    vec3 normal;
    vec4 tangent;
    flat int instanceID;
} vs_out;

layout (set = 0, binding = 0) uniform Camera
{
    scene_camera camera;
};

layout (set = 1, binding = 0) buffer Instances
{
    instance_data instances[MAX_INSTANCE_COUNT];
};

void main(void)
{
    instance_data instance = instances[gl_InstanceIndex];
    mat4 mvp = camera.viewProj * instance.transform;
    vec4 worldPos = mvp * vec4(position, 1);
    gl_Position = worldPos;

    vs_out.worldPosition = worldPos.xyz;
    vs_out.texcoord0 = texcoord0;
    vs_out.normal = normal;
    vs_out.tangent = tangent;
    vs_out.instanceID = gl_InstanceIndex;
}