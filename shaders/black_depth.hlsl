sampler2D ScreenMapSampler : register(s1);
sampler2D DepthMapSampler : register(s2);

#define CLIP_FAR 20480
#define DEPTH_START (1024*12.0/CLIP_FAR)
#define DEPTH_END 1.0

#define BLACK float4(0,0,0,0)

float4 pixelMain
(
    float2 TexCoords : TEXCOORD0,
    float3 LStart : TEXCOORD1,
    float3 LEnd : TEXCOORD2
) : COLOR0
{
    float d = tex2D(DepthMapSampler, TexCoords).r + tex2D(DepthMapSampler, TexCoords).g / 256;
    float l = (d - DEPTH_START) / (DEPTH_END - DEPTH_START);
    const float4 pixel = tex2D(ScreenMapSampler, TexCoords);
    return lerp(pixel, BLACK, clamp(l, 0, 1));
}
