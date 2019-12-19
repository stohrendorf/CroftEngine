uniform sampler2D u_diffuseTexture;
#ifdef WATER
uniform float u_time;
#endif

in vec2 v_texCoord;
in vec3 v_color;
in vec3 v_vertexPos;
#ifdef WATER
in vec3 v_vertexPosWorld;
#endif
in vec3 v_normal;
in vec3 v_ssaoNormal;

layout(location=0) out vec4 out_color;
layout(location=1) out vec3 out_normal;
layout(location=2) out vec3 out_position;

#include "lighting.glsl"

#ifdef WATER
float cellnoise(in vec3 p)
{
    return fract(sin(dot(p, vec3(12.9898,78.233, 54.849))) * 43758.5453) * 2 - 1;
}

vec3 cellnoise3( vec3 p )
{
    return vec3(
        cellnoise(p.xyz),
        cellnoise(p.zxy),
        cellnoise(p.yzx)
    );
}

float voronoi(in vec3 p)
{
    vec3 i_st = floor(p);
    vec3 f_st = fract(p);

    float m_dist = 1.;  // minimum distance

    for (int y= -1; y <= 1; y++) {
        for (int x= -1; x <= 1; x++) {
            for (int z= -1; z <= 1; z++) {
                // Neighbor place in the grid
                vec3 neighbor = vec3(float(x),float(y),float(z));

                // Random position from current + neighbor place in the grid
                vec3 point = cellnoise3(i_st + neighbor);

                // Animate the point
                point = 0.5 + 0.5*sin(u_time*0.005 + 6.2831*point);

                // Vector between the pixel and the point
                vec3 diff = neighbor + point - f_st;

                // Distance to the point
                float dist = length(diff);

                // Keep the closer distance
                m_dist = min(m_dist, dist);
            }
        }
    }

    return pow(m_dist, 2);
}
#endif

void main()
{
    vec4 baseColor = texture2D(u_diffuseTexture, v_texCoord);

    if(baseColor.a < 0.5)
        discard;

    out_color.r = baseColor.r * v_color.r;
    out_color.g = baseColor.g * v_color.g;
    out_color.b = baseColor.b * v_color.b;
    out_color.a = baseColor.a;

#ifdef WATER
    const float Scale1 = 0.003;
    out_color.rgb *= clamp(abs(voronoi(v_vertexPosWorld * Scale1))+0.5, 0, 1);
    const float Scale2 = 0.0011;
    out_color.rgb *= clamp(abs(voronoi(v_vertexPosWorld * Scale2))+0.5, 0, 1);
    #endif

    out_color.rgb *= calc_positional_lighting(v_normal, v_vertexPos) * shadow_map_multiplier();
    out_color.a = 1.0;

    out_normal = v_ssaoNormal;
    out_position = v_vertexPos;
}
