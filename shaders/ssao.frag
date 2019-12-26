uniform sampler2D u_position;
uniform sampler2D u_normals;
uniform sampler2D u_texNoise;

uniform vec3 u_samples[64];
uniform mat4 u_camProjection;

vec2 screenSize = textureSize(u_position, 0);

layout(location=0) out float out_ao;

#include "flat_pipeline_interface.glsl"

void main()
{
    const float radius = 512;
    const float bias = 0.025;

    // get input for SSAO algorithm
    vec3 fragPos = texture(u_position, fpi.texCoord).xyz;
    vec3 normal = normalize(texture(u_normals, fpi.texCoord).xyz);
    // tile noise texture over screen based on screen dimensions divided by noise size
    vec3 randomVec = normalize(texture(u_texNoise, fpi.texCoord * screenSize/4).xyz);
    // create TBN change-of-basis matrix: from tangent-space to view-space
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    // iterate over the sample kernel and calculate occlusion factor
    float occlusion = 0.0;
    for (int i = 0; i < u_samples.length(); ++i)
    {
        // get sample position
        vec3 smp = fragPos + TBN * u_samples[i] * radius;// from tangent to view-space

        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = u_camProjection * vec4(smp, 1.0);// from view to clip-space
        offset.xyz /= offset.w;// perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5;// transform to range 0.0 - 1.0

        // get sample depth
        float sampleDepth = texture(u_position, offset.xy).z;// get depth value of kernel sample

        // range check & accumulate
        if(sampleDepth >= smp.z + bias)
            occlusion += smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
    }
    out_ao = 1.0 - (occlusion / u_samples.length());
}
