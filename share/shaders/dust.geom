#include "transform_interface.glsl"
#include "camera_interface.glsl"
#include "constants.glsl"

const int N = 12;

layout (points) in;
layout (triangle_strip, max_vertices = N) out;

in DustVSInterface {
    float lifetime;
    float size;
} vs[];

out DustFSInterface {
    flat float lifetime;
} fs;

const float ParticleRadius = 0.4;
const float MaxSizeEnd = 2*1024;
const float VanishEnd = 3*1024;

void main()
{
    vec3 pos = gl_in[0].gl_Position.xyz;

    mat4 v = camera.view;
    mat4 mv = camera.view * modelTransform.m;

    float distance = length(vec3(v * vec4(pos, 1.0)));
    if (distance > VanishEnd) {
        return;
    }

    float f = clamp(1-(distance-MaxSizeEnd)/(VanishEnd-MaxSizeEnd), 0, 1);

    mat4 vp = camera.viewProjection;

    // points of the circle are r*f(2*pi*i/N) with f=cos/sin

    float r = ParticleRadius * vs[0].size;
    for (int i=0; i<N/2; ++i) {
        vec3 dx = vec3(mv[0].x, mv[1].x, mv[2].x) * r * cos(2*PI*(i+0.5)/N);
        vec3 dy = vec3(mv[0].y, mv[1].y, mv[2].y) * r * sin(2*PI*(i+0.5)/N);

        gl_Position = vp * vec4(pos + dx + dy, 1);
        fs.lifetime = vs[0].lifetime;
        EmitVertex();
        gl_Position = vp * vec4(pos + dx - dy, 1);
        fs.lifetime = vs[0].lifetime;
        EmitVertex();
    }
    EndPrimitive();
}
