#ifndef funcs_glsl_
#define funcs_glsl_

#define OIT_LAYERS 8

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

float unPremultLinearToSRGB(float c)
{
    if (c < 0.0031308f)
    {
        return c * 12.92f;
    }
    return pow(c, 1.0f / 2.4f) * 1.055f - 0.055f;
}

vec4 unPremultLinearToSRGB(vec4 c)
{
    c.r = unPremultLinearToSRGB(c.r);
    c.g = unPremultLinearToSRGB(c.g);
    c.b = unPremultLinearToSRGB(c.b);
    return c;
}

float unPremultSRGBtoLinear(float c)
{
    if (c < 0.04045f)
    {
        return c / 12.92f;
    }
    return pow((c + 0.055f) / 1.055f, 2.4f);
}

vec4 unPremultSRGBtoLinear(vec4 c)
{
    c.r = unPremultSRGBtoLinear(c.r);
    c.g = unPremultSRGBtoLinear(c.g);
    c.b = unPremultSRGBtoLinear(c.b);
    return c;
}

void doBlend(inout vec4 color, vec4 base)
{
    color.rgb += (1 - color.a) * base.rgb;
    color.a += (1 - color.a) * base.a;
}

void doBlendPacked(inout vec4 color, uint fragment)
{
    vec4 unpackedColor = unpackUnorm4x8(fragment);
    unpackedColor = unPremultSRGBtoLinear(unpackedColor);
    unpackedColor.rgb *= unpackedColor.a;
    doBlend(color, unpackedColor);
}

// bubble sort is fine for small arrays
void small_depth_sort(inout uvec2 array[OIT_LAYERS], int n)
{
#if OIT_LAYERS > 1
    for (int i = (n - 2); i >= 0; --i)
    {
        for (int j = 0; j <= i; ++j)
        {
            float depth = uintBitsToFloat(array[j].g);
            float nextDepth = uintBitsToFloat(array[j + 1].g);

            if (depth >= nextDepth)
            {
                // swap elements
                uvec2 temp = array[j + 1];
                array[j + 1] = array[j];
                array[j] = temp;
            }
        }
    }
#endif
}

uvec2 insertion_sort_with_tail(inout uvec2 array[OIT_LAYERS], uvec2 newItem)
{
    uvec2 newLast = newItem;
    
    // new item is closer than the furthest fragment
    const float newDepth = uintBitsToFloat(newItem.g);
    if (newDepth < uintBitsToFloat(array[OIT_LAYERS - 1].g))
    {
        for (int i = 0; i < OIT_LAYERS; ++i)
        {
            if (newDepth < uintBitsToFloat(array[OIT_LAYERS - 1].g))
            {
                newLast = array[OIT_LAYERS - 1];
                for (int j = OIT_LAYERS - 1; j > i; --j)
                {
                    array[j] = array[j - 1];
                }

                array[i] = newLast;
                break;
            }
        }
    }

    return newLast;
}

#endif // funcs_glsl_