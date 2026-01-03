#include "transform_interface.glsl"
#include "camera_interface.glsl"
#include "constants.glsl"

const int N = 12;

layout (points) in;
layout (triangle_strip, max_vertices = N) out;

in DustVSInterface {
    float alpha;
    float size;
} vs[];

out DustFSInterface {
    float alpha;
} fs;

const float ParticleRadius = 0.4;
const float VanishEnd = 3*1024;

void main()
{
    vec3 pos = gl_in[0].gl_Position.xyz;
    vec4 posView = camera.view * vec4(pos, 1.0);

    if (dot(posView.xyz, posView.xyz) > VanishEnd * VanishEnd) {
        return;
    }

    mat4 mv = camera.view * modelTransform.m;
    mat4 vp = camera.viewProjection;

    float r = ParticleRadius * vs[0].size;
    float alpha = vs[0].alpha;

    vec4 centerClip = vp * vec4(pos, 1.0);
    vec4 axisX = vp * vec4(vec3(mv[0].x, mv[1].x, mv[2].x) * r, 0.0);
    vec4 axisY = vp * vec4(vec3(mv[0].y, mv[1].y, mv[2].y) * r, 0.0);

    for (int i=0; i<N/2; ++i) {
        float angle = 2.0 * PI * (float(i) + 0.5) / float(N);
        float c = cos(angle);
        float s = sin(angle);

        gl_Position = centerClip + (axisX * c + axisY * s);
        fs.alpha = alpha;
        EmitVertex();

        gl_Position = centerClip + (axisX * c - axisY * s);
        fs.alpha = alpha;
        EmitVertex();
    }
    EndPrimitive();
}
