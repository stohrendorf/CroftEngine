uniform sampler2D ScreenMapSampler;
uniform sampler2D DepthMapSampler;

#define CLIP_FAR 20480
#define DEPTH_START (1024*12.0/CLIP_FAR)
#define DEPTH_END 1.0

#define BLACK vec4(0,0,0,0)

void main()
{
    vec2 TexCoords = gl_TexCoord[0].xy;
    float d = texture2D(DepthMapSampler, TexCoords).r + texture2D(DepthMapSampler, TexCoords).g / 256;

    vec4 pixel = texture2D(ScreenMapSampler, TexCoords);

    gl_FragColor = mix(pixel, BLACK, clamp((d - DEPTH_START) / (DEPTH_END - DEPTH_START), 0, 1));
}
