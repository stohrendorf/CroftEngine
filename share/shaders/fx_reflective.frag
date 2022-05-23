#include "flat_pipeline_interface.glsl"
#include "util.glsl"

layout(bindless_sampler) uniform sampler2D u_input;
layout(bindless_sampler) uniform sampler2D u_normal;
layout(bindless_sampler) uniform sampler2D u_reflective;
layout(location=0) out vec3 out_color;

void main()
{
    vec4 reflective = texture(u_reflective, fpi.texCoord).rgba;
    vec2 normal = texture(u_normal, fpi.texCoord).xy;
    vec3 base = texture(u_input, fpi.texCoord).rgb;
    vec3 reflected = texture(u_input, normal*0.5 + vec2(0.5)).rgb * reflective.rgb;
    out_color = mix(base, reflected, reflective.a);
}
