#include "portal_pipeline_interface.glsl"
#include "camera_interface.glsl"
#include "time_uniform.glsl"

#include "noise.glsl"

layout(location=0) out vec2 out_perturb;
layout(location=1) out vec3 out_position;

mat2 rotate2d(in float a){
    float ca = cos(a);
    float sa = sin(a);
    return mat2(ca, -sa, sa, ca);
}

float fbm(in vec2 st) {
    float value = 0.0;
    float amplitude = .5;
    st *= 0.0064;
    for (int i = 0; i < 3; i++) {
        value += amplitude * noise(st);
        st *= 1.4;
        amplitude *= .5;
    }
    return value * .5 + .5;
}
const float TimeMult = 0.0002;
const float TexScale = 2048;

float bumpTex(in vec2 uv, in float time) {
    const float PI = 3.14159265359;
    vec2 coords1 = rotate2d(.9*PI) * uv + time*vec2(0.1, -0.3)*TimeMult;
    vec2 coords2 = rotate2d(.06*PI) * uv - time*vec2(0.1, 0.2)*TimeMult;

    float wave1 = fbm(coords1*vec2(30.0, 20.0));
    float wave2 = fbm(coords2*vec2(30.0, 20.0));

    return pow((wave1 + wave2) * 0.5, 2.0);
}


float bumpFunc(in vec2 st, in float time){
    return bumpTex(st + vec2(bumpTex(st, time)*0.11, 0), time);
}

vec2 bumpMap(in vec2 st, in float time){
    const float eps = 2./TexScale;
    vec2 ff = vec2(
    bumpFunc(st-vec2(eps, 0), time),
    bumpFunc(st-vec2(0, eps), time)
    );

    return (ff-vec2(bumpFunc(st, time)))/eps*0.002;
}

void main()
{
    vec2 uv = ppi.vertexPosWorld.xz / TexScale;
    vec2 bm = bumpMap(uv, u_time);
    vec3 sn = normalize(vec3(bm.x, 1, bm.y));// normal in XZ plane (model space)
    vec2 bm2 = bumpMap(uv*0.1, u_time*0.2);
    sn += normalize(vec3(bm2.x, 1, bm2.y));

    vec4 orig = u_viewProjection * vec4(ppi.vertexPosWorld, 1);
    orig /= orig.w;
    vec4 surface = u_viewProjection * vec4(vec3(sn.x, 0, sn.z) + ppi.vertexPosWorld, 1);
    surface /= surface.w;
    out_perturb = (surface-orig).xy;
    out_position = ppi.vertexPosView;
}
