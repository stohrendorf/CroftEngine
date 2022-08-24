layout(bindless_sampler) uniform sampler2D u_depth;
layout(bindless_sampler) uniform sampler2D u_normals;

layout(location=0) out float out_edge;

#include "flat_pipeline_interface.glsl"

const vec3 Scharr = vec3(47, 162, 47);
const float GradMult = 1.0 / dot(Scharr, vec3(1.0));
const float Threshold = 0.2;

void gather(in sampler2D tex, inout vec3 texels[9], in vec2 texel)
{
    for (int x=-1; x<=1; ++x)
    {
        for (int y=-1; y<=1; ++y)
        {
            texels[(x+1)*3+(y+1)] = texture(tex, fpi.texCoord + vec2(x, y)*texel).xyz;
        }
    }
}

void main()
{
    vec2 texel = 1.0 / vec2(textureSize(u_depth, 0).xy);

    vec3 texels[9];

    #define SCHARR(ACCESSOR) \
    length(vec2(\
        dot(Scharr, vec3(texels[0].ACCESSOR, texels[1].ACCESSOR, texels[2].ACCESSOR)) - dot(Scharr, vec3(texels[6].ACCESSOR, texels[7].ACCESSOR, texels[8].ACCESSOR)), \
        dot(Scharr, vec3(texels[0].ACCESSOR, texels[3].ACCESSOR, texels[6].ACCESSOR)) - dot(Scharr, vec3(texels[2].ACCESSOR, texels[5].ACCESSOR, texels[8].ACCESSOR))) \
)

    gather(u_depth, texels, texel);
    float depthG = SCHARR(x);
    gather(u_normals, texels, texel);
    float normXG = SCHARR(x)*0.5;
    float normYG = SCHARR(y)*0.5;
    float normZG = SCHARR(z)*0.5;

    out_edge = 1.0 - clamp(max(max(max(depthG, normXG), normYG), normZG) * GradMult - Threshold, 0.0, 1.0);
}
