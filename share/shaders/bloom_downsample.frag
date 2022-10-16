#include "flat_pipeline_interface.glsl"

layout(bindless_sampler) uniform sampler2D u_input;

layout(location=0) out vec3 out_tex;

void main()
{
    const vec2 texelSize = 1.0 / vec2(textureSize(u_input, 0));
    const float x = texelSize.x;
    const float y = texelSize.y;

    vec3 a = texture(u_input, vec2(fpi.texCoord.x - 2*x, fpi.texCoord.y + 2*y)).rgb;
    vec3 b = texture(u_input, vec2(fpi.texCoord.x, fpi.texCoord.y + 2*y)).rgb;
    vec3 c = texture(u_input, vec2(fpi.texCoord.x + 2*x, fpi.texCoord.y + 2*y)).rgb;

    vec3 d = texture(u_input, vec2(fpi.texCoord.x - 2*x, fpi.texCoord.y)).rgb;
    vec3 e = texture(u_input, vec2(fpi.texCoord.x, fpi.texCoord.y)).rgb;
    vec3 f = texture(u_input, vec2(fpi.texCoord.x + 2*x, fpi.texCoord.y)).rgb;

    vec3 g = texture(u_input, vec2(fpi.texCoord.x - 2*x, fpi.texCoord.y - 2*y)).rgb;
    vec3 h = texture(u_input, vec2(fpi.texCoord.x, fpi.texCoord.y - 2*y)).rgb;
    vec3 i = texture(u_input, vec2(fpi.texCoord.x + 2*x, fpi.texCoord.y - 2*y)).rgb;

    vec3 j = texture(u_input, vec2(fpi.texCoord.x - x, fpi.texCoord.y + y)).rgb;
    vec3 k = texture(u_input, vec2(fpi.texCoord.x + x, fpi.texCoord.y + y)).rgb;
    vec3 l = texture(u_input, vec2(fpi.texCoord.x - x, fpi.texCoord.y - y)).rgb;
    vec3 m = texture(u_input, vec2(fpi.texCoord.x + x, fpi.texCoord.y - y)).rgb;

    out_tex = e*0.125 + (a+c+g+i)*0.03125 + (b+d+f+h)*0.0625 + (j+k+l+m)*0.125;
}
