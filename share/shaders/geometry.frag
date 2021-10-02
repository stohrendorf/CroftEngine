#include "geometry_pipeline_interface.glsl"
#include "camera_interface.glsl"
#include "time_uniform.glsl"

layout(bindless_sampler) uniform sampler2DArray u_diffuseTextures;
#ifdef IN_WATER
#include "water_caustics.glsl"
#endif

layout(location=0) out vec4 out_color;
layout(location=1) out vec3 out_normal;
layout(location=2) out vec3 out_position;

#include "lighting.glsl"

// from https://core.ac.uk/download/pdf/53544051.pdf
// "A Quadrilateral Rendering Primitive" by Kai Hormann and Marco Tarini
vec4 barycentric(in vec4 wwww)
{
    vec2 v = (gl_FragCoord.xy / camera.viewport.xy) * 2 - 1;
    vec2 s[4];
    vec4 r;
    for (int i=0; i<4; ++i) {
        s[i] = gpi.quadVerts[i].xy - v;
        r[i] = length(s[i]) * sign(wwww[i]);
    }

    vec4 A;
    vec4 D;
    for (int i=0; i<4; ++i) {
        vec2 s1 = s[i];
        vec2 s2 = s[(i+1)%4];
        A[i] = cross(vec3(s1, 0), vec3(s2, 0)).z;
        D[i] = dot(s1, s2);
    }
    vec4 t = (r.xyzw * r.yzwx - D) / A;
    vec4 mu = (t.wxyz + t.xyzw) / r;
    return mu / dot(mu, vec4(1));
}

vec2 barycentricUv()
{
    // section 3.1
    vec4 wwww;
    for (int i=0; i<4; ++i) {
        wwww[i] = gpi.quadVerts[i].z;
    }
    vec4 f = barycentric(wwww)/wwww;
    f /= dot(f, vec4(1));
    vec2 uv = vec2(0);
    for (int i=0; i<4; ++i) {
        uv += f[i] * gpi.quadUvs[i].xy;
    }
    return uv;
}

void main()
{
    out_normal = gpi.hbaoNormal;
    out_position = gpi.vertexPos;

    vec3 finalColor = gpi.color.rgb;
    if (gpi.texCoord.z >= 0) {
        vec2 uv;
        if (gpi.isQuad == 0) {
            uv = gpi.texCoord.xy;
        }
        else {
            uv = barycentricUv();
        }
        vec4 texColor = texture(u_diffuseTextures, vec3(uv, gpi.texCoord.z));
        if (texColor.a < 0.5) {
            discard;
        }
        finalColor *= texColor.rgb;
    }
    else {
        finalColor *= finalColor;
    }

        #ifdef IN_WATER
    finalColor *= water_multiplier(gpi.vertexPosWorld);
    #endif

    out_color = vec4(finalColor * calc_positional_lighting(gpi.vertexNormalWorld, gpi.vertexPosWorld) * shadow_map_multiplier(gpi.vertexNormalWorld), 1.0);
}
