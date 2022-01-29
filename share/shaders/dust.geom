#include "transform_interface.glsl"
#include "camera_interface.glsl"

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in DustVSInterface {
    float lifetime;
    float size;
} vs[];

out DustFSInterface {
    flat float lifetime;
} fs;

const float ParticleRadius = 0.25;

void main()
{
    vec3 pos = gl_in[0].gl_Position.xyz;

    mat4 mv = camera.view * modelTransform.m;
    mat4 vp = camera.viewProjection;

    vec3 dx = vec3(mv[0].x, mv[1].x, mv[2].x) * ParticleRadius * vs[0].size;
    vec3 dy = vec3(mv[0].y, mv[1].y, mv[2].y) * ParticleRadius * vs[0].size;

    gl_Position = vp * vec4(pos - dx - dy, 1);// 1:bottom-left
    fs.lifetime = vs[0].lifetime;
    EmitVertex();
    gl_Position = vp * vec4(pos + dx - dy, 1);// 2:bottom-right
    fs.lifetime = vs[0].lifetime;
    EmitVertex();
    gl_Position = vp * vec4(pos - dx + dy, 1);// 3:top-left
    fs.lifetime = vs[0].lifetime;
    EmitVertex();
    gl_Position = vp * vec4(pos + dx + dy, 1);// 4:top-right
    fs.lifetime = vs[0].lifetime;
    EmitVertex();
    EndPrimitive();
}
