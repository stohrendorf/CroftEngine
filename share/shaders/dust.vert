#include "vtx_input.glsl"
#include "transform_interface.glsl"
#include "time_uniform.glsl"
#include "noise.glsl"
#include "camera_interface.glsl"

out DustFSInterface {
    float alpha;
} fs;

const float ParticleRadius = 0.4;
const float MaxLifetime = 8;
const float MaxDistance = 256;

void main()
{
    vec3 basePos = a_position + vec3(modelTransform.m[3].xyz);

    vec3 n = snoise3(basePos * 0.0013 + 17.0);
    vec3 n2 = snoise3(basePos.zxy * 0.0017 + 31.0);

    float randS = pow(0.5, 0.5 * n.y + 1.0) * pow(0.5, 1.0 - (0.5 * n.y + 1.0));
    float size = 0.5 + randS * 4.0;

    float particleMaxLifetime = (n2.x * 0.5 + 1.2) * MaxLifetime;

    float t = mod(TimeSeconds, particleMaxLifetime);
    float t0 = TimeSeconds - t;
    vec3 pnoise = snoise3(basePos * 0.0021 + vec3(t0 * 0.3, t0 * 0.2, t0 * 0.1));
    vec3 normal = snoise3(basePos.zyx * 0.0011 + pnoise);
    float dist = snoise3(basePos.zyx * 0.0015 - pnoise).x * MaxDistance;

    float lifetime = t / particleMaxLifetime;
    fs.alpha = clamp(min(lifetime, 1.0 - lifetime) * 3.0, 0.0, 1.0) * 0.3;
    vec3 pos = a_position + normal * dist * (t + pnoise.y) / particleMaxLifetime;

    vec4 worldPos = modelTransform.m * vec4(pos, 1.0);
    vec4 viewPos = camera.view * worldPos;

    if (viewPos.z >= -camera.nearPlane) {
        gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
        return;
    }

    gl_Position = camera.projection * viewPos;
    gl_PointSize = (camera.viewport.y * camera.projection[1][1] * ParticleRadius * size) / gl_Position.w;
}
