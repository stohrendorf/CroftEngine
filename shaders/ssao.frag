uniform sampler2D u_position;
uniform sampler2D u_normals;

uniform vec3 u_samples[16];

layout(location=0) out float out_ao;

#include "flat_pipeline_interface.glsl"
#include "camera_interface.glsl"
#include "noise.glsl"

void main()
{
    const float radius = 32;
    const float bias = 16;

    // get input for SSAO algorithm
    vec3 fragPos = texture(u_position, fpi.texCoord).xyz;
    vec3 normal = normalize(texture(u_normals, fpi.texCoord).xyz);
    // tile noise texture over screen based on screen dimensions divided by noise size
    vec3 randomVec = normalize(snoise3(vec3(fpi.texCoord, normal.x)));
    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal) * radius;
    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for (int i = 0; i < u_samples.length(); ++i)
    {
        // get sample position
        vec3 smp = TBN * u_samples[i] + fragPos;// from tangent to view-space

        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = u_projection * vec4(smp, 1.0);// from view to clip-space
        offset /= offset.w;// perspective divide
        offset = offset * vec4(0.5) + vec4(0.5);// transform to range 0.0 - 1.0

        // get sample depth
        float sampleDepth = dot(textureGather(u_position, offset.xy, 2), vec4(.25));

        // range check & accumulate
        if (sampleDepth >= smp.z + bias)
        {
            occlusion += 1.0 / abs(fragPos.z - sampleDepth);
        }
    }
    out_ao = pow(1.0 - clamp(occlusion / u_samples.length(), 0, 1), 64);
}
