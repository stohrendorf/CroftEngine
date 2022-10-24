layout(bindless_sampler) uniform sampler2D u_edges;

layout(location=0) out float out_dilated;

#include "flat_pipeline_interface.glsl"

void main()
{
    vec2 texel = 1.0 / vec2(textureSize(u_edges, 0).xy);

    float value = 1.0;
    for (int x=-1; x<=1; ++x)
    {
        for (int y=-1; y<=1; ++y)
        {
            float e = texture(u_edges, fpi.texCoord + vec2(x, y)*texel).x;
            value = min(value, e);
        }
    }
    out_dilated = value < 0.9 ? 0.0 : (value-0.9) * 10.0;
}
