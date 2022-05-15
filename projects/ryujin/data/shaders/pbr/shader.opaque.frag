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

vec3 get_normal_from_map()
{
    uint matId = instances[fs_in.instanceID].material;
    material mat = materials[matId];
    if (mat.normal >= texturesLoaded)
    {
        return fs_in.normal;
    }

    vec3 tangentSpaceNormal = 2.0 * texture(textures[mat.normal], fs_in.texcoord0).rgb - 1.0;
    vec3 q1 = dFdx(fs_in.worldPosition);
    vec3 q2 = dFdy(fs_in.worldPosition);
    vec2 st1 = dFdx(fs_in.texcoord0);
    vec2 st2 = dFdy(fs_in.texcoord0);
    vec3 n = fs_in.normal;
    vec3 t = fs_in.tangent;
    vec3 b = fs_in.bitangent;
    
    mat3 tbn = mat3(t, b, n);
    vec3 worldNormal = normalize(tbn * tangentSpaceNormal);

    return worldNormal;
}

float ggx_dist(vec3 n, vec3 h, float rough)
{
    float a = rough * rough;
    float a2 = a * a;
    float nDotH = max(dot(n, h), 0.0);
    float nDotH2 = nDotH * nDotH;
    float denom = (nDotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return a2 / denom;
}

float ggx_geom_schlick(float nDotV, float rough)
{
    float r = rough + 1.0;
    float k = (r * r) / 8.0;
    float denom = nDotV * (1.0 - k) + k;
    return nDotV / denom;
}

float geom_smith(vec3 n, vec3 v, vec3 l, float rough)
{
    float nDotV = max(dot(n, v), 0.0);
    float nDotL = max(dot(n, l), 0.0);
    return ggx_geom_schlick(nDotV, rough) * ggx_geom_schlick(nDotL, rough);
}

vec3 fresnel_schlick(float cosTheta, vec3 f0)
{
    return f0 + (1.0 - f0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec2 get_metal_rough(uint id)
{
    if (id >= texturesLoaded)
    {
        return vec2(0);
    }
    return texture(textures[id], fs_in.texcoord0).bg;
}

float get_ao(uint id)
{
    if (id >= texturesLoaded)
    {
        return 0.1;
    }
    return texture(textures[id], fs_in.texcoord0).r;
}

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
        scene_camera cam = cameras[activeCamera];

        vec4 baseAlbedo = texture(textures[mat.albedo], fs_in.texcoord0);
        vec3 albedo = pow(baseAlbedo.xyz, vec3(2.2));
        vec2 metalRough = get_metal_rough(mat.metallicRoughness);
        float metal = metalRough.r;
        float rough = metalRough.g;
        float ao = get_ao(mat.ambientOcclusion);
        
        vec3 normalFromTexture = get_normal_from_map();
        vec3 viewVector = normalize(cam.position - fs_in.worldPosition);

        vec3 reflectance = vec3(0.04);
        reflectance = mix(reflectance, albedo, metal);

        vec3 lo = vec3(0.0);

        // compute directional lighting
        vec3 sunLightVector = -normalize(lighting.sun.direction.xyz);
        vec3 halfV = normalize(sunLightVector + viewVector);
        vec3 radiance = lighting.sun.color.rgb;

        float ndf = ggx_dist(normalFromTexture, halfV, rough);
        float g = geom_smith(normalFromTexture, viewVector, sunLightVector, rough);
        vec3 f = fresnel_schlick(max(dot(halfV, viewVector), 0.0), reflectance);

        vec3 numerator = ndf * g * f;
        float denom = 4.0 * max(dot(normalFromTexture, viewVector), 0.0) * max(dot(normalFromTexture, sunLightVector), 0.0) + 0.0001; // bias to prevent divide by zero
        vec3 specular = numerator / denom;

        vec3 ks = f;
        vec3 kd = vec3(1.0) - ks;
        kd *= (1.0 - metal);

        float nDotL = max(dot(normalFromTexture, sunLightVector), 0.0);
        lo += (kd * albedo / PI + specular) * radiance * nDotL;

        // compute ambient
        vec3 ambient = vec3(0.03) * albedo * ao;
        vec3 color = ambient + lo;

        color = color / (color + vec3(1.0));
        color = pow(color, vec3(1.0 / 2.2));
        fragColor = vec4(color, 1.0);
    }
}