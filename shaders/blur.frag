#include "flat_pipeline_interface.glsl"

uniform sampler2D u_input;
layout(location=0) out float out_tex;

const int BlurExtent = BLUR_EXTENT;
const float BlurSize = BlurExtent*2 + 1;

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(u_input, 0));
    float result = 0.0;
    if (BLUR_DIR == 0)
    {
        for (int x = -BlurExtent; x <= BlurExtent; ++x)
        {
            for (int y = -BlurExtent; y <= BlurExtent; ++y)
            {
                vec2 offset = vec2(float(x), float(y)) * texelSize;
                result += texture(u_input, fpi.texCoord + offset).r;
            }
        }
        out_tex = result / (BlurSize * BlurSize);
    }
    else if (BLUR_DIR == 1)
    {
        for (int x = -BlurExtent; x <= BlurExtent; ++x)
        {
            vec2 offset = vec2(float(x), 0) * texelSize;
            result += texture(u_input, fpi.texCoord + offset).r;
        }
        out_tex = result / BlurSize;
    }
    else if (BLUR_DIR == 2)
    {
        for (int y = -BlurExtent; y <= BlurExtent; ++y)
        {
            vec2 offset = vec2(0, float(y)) * texelSize;
            result += texture(u_input, fpi.texCoord + offset).r;
        }
        out_tex = result / BlurSize;
    }
}
