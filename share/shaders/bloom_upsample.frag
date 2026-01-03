#include "flat_pipeline_interface.glsl"

layout(bindless_sampler) uniform sampler2D u_input;

layout(location=0) out vec3 out_tex;

void main()
{
    const vec2 texelSize = 1.0 / vec2(textureSize(u_input, 0));
    const float x = texelSize.x * 0.5;
    const float y = texelSize.y * 0.5;

    vec3 a = texture(u_input, fpi.texCoord + vec2(-x, y)).rgb;
    vec3 b = texture(u_input, fpi.texCoord + vec2(x, y)).rgb;
    vec3 c = texture(u_input, fpi.texCoord + vec2(-x, -y)).rgb;
    vec3 d = texture(u_input, fpi.texCoord + vec2(x, -y)).rgb;

    out_tex = (a + b + c + d) * 0.25;
}
