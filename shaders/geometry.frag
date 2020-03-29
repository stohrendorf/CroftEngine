#include "geometry_pipeline_interface.glsl"
#include "camera_interface.glsl"
#include "time_uniform.glsl"

layout(binding=4) uniform sampler2DArray u_diffuseTextures;
#ifdef WATER
#include "water.glsl"
#endif

layout(location=0) out vec4 out_color;
layout(location=1) out vec3 out_normal;
layout(location=2) out vec3 out_position;

#include "lighting.glsl"

void main()
{
    vec4 baseColor;
    if (gpi.texIndex >= 0) {
        baseColor = texture(u_diffuseTextures, vec3(gpi.texCoord, gpi.texIndex));
        if (baseColor.a < 0.5) {
            discard;
        }
    }
    else {
        baseColor = vec4(gpi.color, 1);
    }

    vec3 finalColor = baseColor.rgb * gpi.color.rgb;

    #ifdef WATER
    finalColor *= water_multiplier(gpi.vertexPosWorld);
    #endif

    //out_color.rgb = finalColor * calc_positional_lighting(gpi.normal, gpi.vertexPos) * shadow_map_multiplier();
    out_color.rgb = calc_positional_pbr_lighting(u_inverseView[3].xyz, gpi.normal, gpi.vertexPos, finalColor) * shadow_map_multiplier(gpi.normal);
    out_color.a = 1.0;

    out_normal = gpi.ssaoNormal;
    out_position = gpi.vertexPos;
}
