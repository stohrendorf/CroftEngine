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

// from https://core.ac.uk/download/pdf/53544051.pdf
// "A Quadrilateral Rendering Primitive" by Kai Hormann and Marco Tarini
vec4 barycentric(out vec4 wwww)
{
    vec2 v = (gl_FragCoord.xy / u_screenSize.xy) * 2 - 1;
    vec2 s1 = gpi.quadV1.xy - v;
    vec2 s2 = gpi.quadV2.xy - v;
    vec2 s3 = gpi.quadV3.xy - v;
    vec2 s4 = gpi.quadV4.xy - v;

    vec4 A = vec4(
    cross(vec3(s1, 0), vec3(s2, 0)).z,
    cross(vec3(s2, 0), vec3(s3, 0)).z,
    cross(vec3(s3, 0), vec3(s4, 0)).z,
    cross(vec3(s4, 0), vec3(s1, 0)).z
    );
    vec4 D = vec4(
    dot(s1, s2),
    dot(s2, s3),
    dot(s3, s4),
    dot(s4, s1)
    );
    wwww = vec4(
    gpi.quadV1.z,
    gpi.quadV2.z,
    gpi.quadV3.z,
    gpi.quadV4.z
    );
    vec4 r = vec4(
    length(s1),
    length(s2),
    length(s3),
    length(s4)
    ) * sign(wwww);
    vec4 t = (r.xyzw * r.yzwx - D) / A;
    vec4 mu = (t.wxyz + t.xyzw) / r;
    return mu / dot(mu, vec4(1));
}

vec2 barycentricUv()
{
    // section 3.1
    vec4 wwww;
    vec4 lambda = barycentric(wwww);
    vec4 f = lambda/wwww;
    float sumW = dot(f, vec4(1));
    f /= sumW;
    vec2 uv = f.x * gpi.quadUv1.xy
    + f.y * gpi.quadUv2.xy
    + f.z * gpi.quadUv3.xy
    + f.w * gpi.quadUv4.xy;
    return uv;
}

void main()
{
    vec4 baseColor;
    if (gpi.texIndex >= 0) {
        vec2 uv;
        if (gpi.isQuad == 0) {
            uv = gpi.texCoord;
        }
        else {
            uv = barycentricUv();
        }
        baseColor = texture(u_diffuseTextures, vec3(uv, gpi.texIndex));
        if (baseColor.a < 0.5) {
            discard;
        }
    }
    else {
        baseColor = gpi.color;
    }

    vec3 finalColor = baseColor.rgb * gpi.color.rgb;

    #ifdef WATER
    finalColor *= water_multiplier(gpi.vertexPosWorld);
    #endif

    out_color.rgba = vec4(finalColor * calc_positional_lighting(gpi.normal, gpi.vertexPosWorld) * shadow_map_multiplier(gpi.normal), 1.0);
    out_normal = gpi.ssaoNormal;
    out_position = gpi.vertexPos;
}
