#include "vtx_input.glsl"
#include "transform_interface.glsl"
#include "time_uniform.glsl"
#include "noise.glsl"

out DustVSInterface {
    float alpha;
    float size;
} vs;

const float MaxLifetime = 8;
const float MaxDistance = 256;

void main()
{
    vec3 n = snoise3(a_position.xyz);
    vec3 n2 = snoise3(a_position.zxy);

    float randS = pow(0.5, 0.5 * n.y + 1) * pow(0.5, 1 - (0.5 * n.y + 1));
    vs.size = 0.5 + randS*4;

    float particleMaxLifetime = (n2.x * 0.5 + 1.2) * MaxLifetime;

    float t = mod(TimeSeconds, particleMaxLifetime);
    float t0 = TimeSeconds - t;
    vec3 pnoise = snoise3(a_position.xyz + vec3(t0*3, t0*2, t0));
    vec3 normal = snoise3(a_position.zyx + pnoise);
    float distance = snoise3(a_position.zyx - pnoise).x * MaxDistance;

    float lifetime = t / particleMaxLifetime;
    vs.alpha = clamp(min(lifetime, 1.0-lifetime) * 3.0, 0.0, 1.0) * 0.3;
    vec3 pos = a_position + normal * distance * (t+pnoise.y) / particleMaxLifetime;
    gl_Position = modelTransform.m * vec4(pos, 1);
}
