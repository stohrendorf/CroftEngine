uniform sampler2D u_ao;

#include "flat_pipeline_interface.glsl"

layout(location=0) out float out_ao;

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(u_ao, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x)
    {
        for (int y = -2; y < 2; ++y)
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(u_ao, fpi.texCoord + offset).r;
        }
    }
    out_ao = result / (4.0 * 4.0);
}
