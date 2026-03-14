#include "flat_pipeline_interface.glsl"
#include "fx_input.glsl"
#include "util.glsl"
#include "constants.glsl"

const vec2 PixelRes = vec2(480.0, 360.0) * 2;
const vec2 PixelSize = 1.0 / PixelRes;

void main()
{
    vec2 uv = fpi.texCoord;
    vec2 res = textureSize(u_input, 0).xy;

    vec2 center = uv - 0.5;
    center *= 0.96;
    uv = center + 0.5;

    float dist = dot(center, center);
    vec2 distorted = uv + center * dist * 0.08;

    distorted = clamp(distorted, 0.0, 1.0);

    vec2 pixelated = floor(distorted * PixelRes) / PixelRes;

    vec3 col = vec3(0.0);

    col += texture(u_input, clamp(pixelated, 0.0, 1.0)).rgb * 0.4;

    col += texture(u_input, clamp(pixelated + vec2(PixelSize.x * 0.5, 0.0), 0.0, 1.0)).rgb * 0.15;
    col += texture(u_input, clamp(pixelated - vec2(PixelSize.x * 0.5, 0.0), 0.0, 1.0)).rgb * 0.15;

    col += texture(u_input, clamp(pixelated + vec2(0.0, PixelSize.y * 0.5), 0.0, 1.0)).rgb * 0.1;
    col += texture(u_input, clamp(pixelated - vec2(0.0, PixelSize.y * 0.5), 0.0, 1.0)).rgb * 0.1;

    col += texture(u_input, clamp(pixelated + vec2(PixelSize.x * 0.35, PixelSize.y * 0.35), 0.0, 1.0)).rgb * 0.025;
    col += texture(u_input, clamp(pixelated - vec2(PixelSize.x * 0.35, PixelSize.y * 0.35), 0.0, 1.0)).rgb * 0.025;
    col += texture(u_input, clamp(pixelated + vec2(PixelSize.x * 0.35, -PixelSize.y * 0.35), 0.0, 1.0)).rgb * 0.025;
    col += texture(u_input, clamp(pixelated + vec2(-PixelSize.x * 0.35, PixelSize.y * 0.35), 0.0, 1.0)).rgb * 0.025;

    col.r += texture(u_input, clamp(pixelated + vec2(PixelSize.x * 0.7, 0.0), 0.0, 1.0)).r * 0.03;
    col.b += texture(u_input, clamp(pixelated - vec2(PixelSize.x * 0.7, 0.0), 0.0, 1.0)).b * 0.03;

    vec2 pixelPos = uv * res;
    float mask = 1.0;
    float x = fract(pixelPos.x * 0.3333);// Every 3rd pixel
    if (x < 0.333){
        mask = 0.88;// R subpixel dimmer
    }
    else if (x < 0.666) {
        mask = 1.0;// G subpixel normal
    }
    else {
        mask = 0.85;// B subpixel dimmer
    }

    col *= mask;

    float scanline = sin(pixelPos.y * PI * 0.5) * 0.08 + 0.92;
    col *= scanline;

    col *= vec3(1.08, 1.03, 1.0);

    out_color = col;
}
