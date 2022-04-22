#version 460

#include "funcs.glsl"
#include "structs.glsl"

layout (location = 0) in mediump vec3 position;
layout (location = 1) in mediump vec4 encodedTbn;
layout (location = 2) in mediump vec2 texcoord0;

layout (location = 0) out VS_OUT
{
    vec3 worldPosition;
    vec2 texcoord0;
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    flat int instanceID;
} vs_out;

layout (set = 0, binding = 0) uniform Camera
{
    scene_camera cameras[MAX_CAMERA_COUNT];
};

layout (set = 1, binding = 0) buffer Instances
{
    instance_data instances[MAX_INSTANCE_COUNT];
};

layout (push_constant) uniform constants
{
    uint activeCamera;
};

void main(void)
{
    scene_camera camera = cameras[activeCamera];

    instance_data instance = instances[gl_InstanceIndex];
    mat4 mvp = camera.viewProj * instance.transform;
    vec4 worldPos = mvp * vec4(position, 1);
    gl_Position = worldPos;
    vs_out.worldPosition = worldPos.xyz;
    vs_out.texcoord0 = texcoord0;
    decode_tbn_quaternion(encodedTbn, vs_out.normal, vs_out.tangent, vs_out.bitangent);
    vs_out.normal = mat3(transpose(inverse(instance.transform))) * vs_out.normal;
    vs_out.instanceID = gl_InstanceIndex;
}