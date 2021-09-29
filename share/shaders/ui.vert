#include "vtx_input.glsl"
#include "ui_pipeline_interface.glsl"
#include "camera_interface.glsl"

float toLinear(in float srgb)
{
    return srgb <= 0.04045
    ? srgb / 12.92
    : pow((srgb + 0.055) / 1.055, 2.4);
}

vec4 toLinear(in vec4 srgb)
{
    return vec4(
    toLinear(srgb.r),
    toLinear(srgb.g),
    toLinear(srgb.b),
    srgb.a
    );
}

void main()
{
    vec2 p = (a_position.xy / camera.viewport.xy) * 2 - 1;
    gl_Position = vec4(p.x, -p.y, 0, 1);
    upi.texCoord = a_texCoord;
    upi.topLeft = toLinear(a_colorTopLeft);
    upi.topRight = toLinear(a_colorTopRight);
    upi.bottomLeft = toLinear(a_colorBottomLeft);
    upi.bottomRight = toLinear(a_colorBottomRight);
}
