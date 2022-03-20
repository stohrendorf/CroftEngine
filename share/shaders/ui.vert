#include "vtx_input.glsl"
#include "ui_pipeline_interface.glsl"
#include "camera_interface.glsl"
#include "util.glsl"

void main()
{
    vec2 p = (a_position.xy / camera.viewport.xy) * 2 - 1;
    gl_Position = vec4(p.x, -p.y, 0, 1);
    upi.texCoord = a_texCoord;
    upi.topLeft = toLinear(a_colorTopLeft);
    upi.topRight = toLinear(a_colorTopRight);
    upi.bottomLeft = toLinear(a_colorBottomLeft);
    upi.bottomRight = toLinear(a_colorBottomRight);
    upi.alpha = a_color.a;
}
