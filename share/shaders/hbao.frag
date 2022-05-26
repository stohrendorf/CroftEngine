layout(bindless_sampler) uniform sampler2D u_position;
layout(bindless_sampler) uniform sampler2D u_normals;

uniform vec3 u_samples[16];

layout(location=0) out float out_ao;

#include "flat_pipeline_interface.glsl"
#include "camera_interface.glsl"
#include "noise.glsl"
#include "constants.glsl"

vec3 angleAxis(vec3 v, vec3 axis, float theta)
{
    return cross(axis, v) * sin(theta) + mix(axis * dot(axis, v), v, cos(theta));
}

void main()
{
    const float Radius = 32;
    const float Bias = 0.025;
    const int Dirs = 8;
    const float DirRotation = 2*PI/Dirs;
    const int Steps = 8;

    vec3 fragPos = texture(u_position, fpi.texCoord).xyz;
    float stepSize = Radius / (fragPos.z*0.0001) / float(Steps+1);
    float stepSizes[Dirs];
    for (int i = 0; i < Dirs; ++i) {
        stepSizes[i] = stepSize * snoise(fpi.texCoord + vec2(i, 0)) + stepSize;
    }

    vec3 normal = texture(u_normals, fpi.texCoord).xyz;
    vec3 baseTangent;
    if (abs(normal.z) > 1e-4 || abs(normal.x) > 1e-4) {
        baseTangent = angleAxis(normalize(vec3(normal.z, 0, -normal.x)), normal, snoise(fpi.texCoord));
    }
    else {
        baseTangent = angleAxis(normalize(vec3(normal.y, -normal.x, 0)), normal, snoise(fpi.texCoord));
    }

    vec3 tangents[Dirs];
    for (int i = 0; i < Dirs; ++i) {
        tangents[i] = stepSizes[i] * angleAxis(baseTangent, normal, i*DirRotation);
    }

    float occlusion = 0.0;
    for (int i = 0; i < Dirs; ++i)
    {
        vec3 tangent = tangents[i];
        vec3 d = fragPos;
        for (int j = 0; j < Steps; ++j)
        {
            d += tangent;
            vec4 offset = camera.projection * vec4(d, 1.0);
            vec3 k = texture(u_position, (offset.xy / offset.w) * vec2(0.5) + vec2(0.5)).xyz;
            vec3 vk = k - fragPos;
            float lvk = 1.0 / length(vk);
            float w = min(Radius * lvk, 1);
            occlusion += w * max(dot(vk, normal) * lvk - Bias, 0);
        }
    }
    out_ao = pow(1 - occlusion / (Steps*Dirs), 1.5);
}
