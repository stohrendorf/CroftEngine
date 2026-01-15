layout(bindless_sampler) uniform sampler2D u_position;
layout(bindless_sampler) uniform sampler2D u_normals;

layout(location=0) out float out_ao;

#include "flat_pipeline_interface.glsl"
#include "camera_interface.glsl"
#include "noise.glsl"
#include "constants.glsl"

void main()
{
    const float Radius = 32.0;
    const float Bias = 0.025;
    const int Dirs = 8;
    const int Steps = 8;
    const float DirRotation = 2.0*PI/float(Dirs);
    const float invTotalSamples = 1.0 / float(Steps * Dirs);

    vec3 fragPos = texture(u_position, fpi.texCoord).xyz;
    vec3 normal = texture(u_normals, fpi.texCoord).xyz;

    float rcpDepthFactor = 1e4 / fragPos.z;
    float baseStepSize = Radius * rcpDepthFactor / float(Steps + 1);

    vec3 baseTangent;
    if (abs(normal.z) > 1e-4 || abs(normal.x) > 1e-4) {
        baseTangent = normalize(vec3(normal.z, 0, -normal.x));
    } else {
        baseTangent = normalize(vec3(normal.y, -normal.x, 0));
    }

    float noiseAngle = snoise(fpi.texCoord);
    float sn = sin(noiseAngle);
    float cs = cos(noiseAngle);
    vec3 rotatedTangent = baseTangent * cs + cross(normal, baseTangent) * sn;

    float occlusion = 0.0;

    for (int dir = 0; dir < Dirs; ++dir)
    {
        float angle = float(dir) * DirRotation;
        float s = sin(angle);
        float c = cos(angle);
        vec3 tangent = rotatedTangent * c + cross(normal, rotatedTangent) * s;

        float dirNoise = snoise(fpi.texCoord + vec2(dir, 0));
        float stepSize = baseStepSize * dirNoise + baseStepSize;
        vec3 stepVec = tangent * stepSize;

        vec3 samplePos = fragPos;

        for (int step = 0; step < Steps; ++step)
        {
            samplePos += stepVec;

            vec4 projected = camera.projection * vec4(samplePos, 1.0);
            vec2 screenUV = projected.xy / projected.w * 0.5 + 0.5;

            vec3 occluderPos = texture(u_position, screenUV).xyz;
            vec3 occluderVec = occluderPos - fragPos;

            float occluderDistSq = dot(occluderVec, occluderVec);
            float rcpOccluderDist = inversesqrt(occluderDistSq);

            float attenuation = min(Radius * rcpOccluderDist, 1.0);

            float normalDot = dot(occluderVec, normal) * rcpOccluderDist;
            occlusion += attenuation * max(normalDot - Bias, 0.0);
        }
    }

    out_ao = pow(1.0 - occlusion * invTotalSamples, 1.5);
}
