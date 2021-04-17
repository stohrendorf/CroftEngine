#include "geometry_pipeline_interface.glsl"
#include "camera_interface.glsl"
#include "time_uniform.glsl"

layout(binding=4) uniform sampler2DArray u_diffuseTextures;
#ifdef WATER
#include "water_caustics.glsl"
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
    vec2 s[4];
    for (int i=0; i<4; ++i) {
        s[i] = gpi.quadVerts[i].xy - v;
    }

    vec4 A;
    for (int i=0; i<4; ++i) {
        A[i] = cross(vec3(s[i], 0), vec3(s[(i+1)%4], 0)).z;
    }
    vec4 D;
    for (int i=0; i<4; ++i) {
        D[i] = dot(s[i], s[(i+1)%4]);
    }
    for (int i=0; i<4; ++i) {
        wwww[i] = gpi.quadVerts[i].z;
    }
    vec4 r;
    for (int i=0; i<4; ++i) {
        r[i] = length(s[i]);
    }
    r *= sign(wwww);
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
    vec2 uv = vec2(0);
    for (int i=0; i<4; ++i) {
        uv += f[i] * gpi.quadUvs[i].xy;
    }
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
    out_normal = gpi.hbaoNormal;
    out_position = gpi.vertexPos;
}
