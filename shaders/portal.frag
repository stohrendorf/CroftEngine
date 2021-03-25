#include "portal_pipeline_interface.glsl"
#include "camera_interface.glsl"
#include "time_uniform.glsl"

#include "noise.glsl"

layout(location=0) out vec2 out_perturb;

mat2 rotate2d(in float a){
    float ca = cos(a);
    float sa = sin(a);
    return mat2(ca, -sa, sa, ca);
}

float fbm(in vec2 st) {
    float value = 0.0;
    float amplitude = .5;
    st *= 0.0004;
    for (int i = 0; i < 6; i++) {
        value += amplitude * noise(st);
        st *= 1.4;
        amplitude *= .5;
    }
    return value * .5 + .5;
}
const float TimeMult = 0.0002;
const float TexScale = 2048;

float bumpTex(in vec2 uv) {
    const float PI = 3.14159265359;
    vec2 coords1 = rotate2d(.9*PI) * uv + u_time*vec2(0.1, -0.3)*TimeMult;
    vec2 coords2 = rotate2d(.06*PI) * uv - u_time*vec2(0.1, 0.2)*TimeMult;

    float wave1 = fbm(coords1*vec2(30.0, 20.0));
    float wave2 = fbm(coords2*vec2(30.0, 20.0));

    return pow((wave1 + wave2) / 2.0, 2.0);
}


float bumpFunc(in vec2 st){
    return bumpTex(st + vec2(bumpTex(st)*0.11, 0));
}

vec2 bumpMap(in vec2 st){
    const float eps = 2./TexScale;
    vec2 ff = vec2(
    bumpFunc(st-vec2(eps, 0)),
    bumpFunc(st-vec2(0, eps))
    );

    return (ff-vec2(bumpFunc(st)))/eps*0.002;
}

void main()
{
    vec2 uv = ppi.vertexPosWorld.xz / TexScale;
    vec2 bm = bumpMap(uv);
    vec3 sn = normalize(vec3(bm.x, 1, bm.y)); // normal in XZ plane (model space)

    const float IOR = 1.3;
    vec4 orig = u_viewProjection * vec4(ppi.vertexPosWorld, 1);
    orig /= orig.w;
    vec4 surface = u_viewProjection * vec4(vec3(sn.x, 0, sn.z) + ppi.vertexPosWorld, 1);
    surface /= surface.w;
    out_perturb = (surface-orig).xy;
}
