#include "geometry_pipeline_interface.glsl"

uniform sampler2D u_diffuseTexture;
#ifdef WATER
uniform float u_time;
#include "water.glsl"
#endif

layout(location=0) out vec4 out_color;
layout(location=1) out vec3 out_normal;
layout(location=2) out vec3 out_position;

#include "lighting.glsl"

void main()
{
    vec4 baseColor = texture(u_diffuseTexture, gpi.texCoord);

    if (baseColor.a < 0.5) {
        discard;
    }

    vec3 finalColor = baseColor.rgb * gpi.color.rgb;

    #ifdef WATER
    finalColor *= water_multiplier(gpi.vertexPosWorld);
    #endif

    out_color.rgb = finalColor * calc_positional_lighting(gpi.normal, gpi.vertexPos) * shadow_map_multiplier();
    out_color.a = 1.0;

    out_normal = gpi.ssaoNormal;
    out_position = gpi.vertexPos;
}
