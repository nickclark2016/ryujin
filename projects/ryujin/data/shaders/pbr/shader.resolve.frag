#version 460

#include "funcs.glsl"
#include "structs.glsl"

layout (location = 0) out vec4 fragColor;

layout (set = 2, binding = 0, rgba32ui) uniform coherent uimageBuffer aBuffer;
layout (set = 2, binding = 1, r32ui) uniform coherent uimage2DArray imageAux;

void main(void)
{
    uvec2 array[OIT_LAYERS];
    vec4 color = vec4(0);
    int fragments = 0;

    const ivec3 coord = ivec3(gl_FragCoord.xy, gl_SampleID);
    uint startOffset = imageLoad(imageAux, coord).r;

    while (startOffset != uint(0) && fragments < OIT_LAYERS)
    {
        // color, depth, 0, next index
        const uvec4 stored = imageLoad(aBuffer, int(startOffset));
        array[fragments] = stored.rg;
        ++fragments;

        startOffset = stored.a;
    }

    // sort fragments
    small_depth_sort(array, fragments);

    vec4 tailColor = vec4(0);

    while (startOffset != uint(0))
    {
        uvec4 stored = imageLoad(aBuffer, int(startOffset));
        uvec2 tail = insertion_sort_with_tail(array, stored.rg);
        doBlendPacked(tailColor, tail.r);
        startOffset = stored.a;
    }

    vec4 colorSum = vec4(0);

    for (int i = 0; i < fragments; ++i)
    {
        doBlendPacked(colorSum, array[i].x);
    }

    doBlend(colorSum, tailColor);
    fragColor = colorSum;
}