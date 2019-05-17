#include "util.glsl"

#ifndef FXAA_QUALITY_PRESET
    #define FXAA_QUALITY_PRESET 39
#endif

#if (FXAA_QUALITY_PRESET == 10)
    const float FxaaQualityP[3] = { 1.5, 3, 12 };
#endif
#if (FXAA_QUALITY_PRESET == 11)
    const float FxaaQualityP[4] = { 1.0, 1.5, 3, 12 };
#endif
#if (FXAA_QUALITY_PRESET == 12)
    const float FxaaQualityP[5] = { 1.0, 1.5, 2, 4, 12 };
#endif
#if (FXAA_QUALITY_PRESET == 13)
    const float FxaaQualityP[6] = { 1.0, 1.5, 2, 2, 4, 12 };
#endif
#if (FXAA_QUALITY_PRESET == 14)
    const float FxaaQualityP[7] = { 1.0, 1.5, 2, 2, 2, 4, 12 };
#endif
#if (FXAA_QUALITY_PRESET == 15)
    const float FxaaQualityP[8] = { 1.0, 1.5, 2, 2, 2, 2, 4, 12 };
#endif
#if (FXAA_QUALITY_PRESET == 20)
    const float FxaaQualityP[3] = { 1.5, 2, 8 };
#endif
#if (FXAA_QUALITY_PRESET == 21)
    const float FxaaQualityP[4] = { 1, 1.5, 2, 8 };
#endif
#if (FXAA_QUALITY_PRESET == 22)
    const float FxaaQualityP[5] = { 1, 1.5, 2, 2, 8 };
#endif
#if (FXAA_QUALITY_PRESET == 23)
    const float FxaaQualityP[6] = { 1, 1.5, 2, 2, 2, 8 };
#endif
#if (FXAA_QUALITY_PRESET == 24)
    const float FxaaQualityP[7] = { 1, 1.5, 2, 2, 2, 3, 8 };
#endif
#if (FXAA_QUALITY_PRESET == 25)
    const float FxaaQualityP[8] = { 1, 1.5, 2, 2, 2, 2, 4, 8 };
#endif
#if (FXAA_QUALITY_PRESET == 26)
    const float FxaaQualityP[9] = { 1, 1.5, 2, 2, 2, 2, 2, 4, 8 };
#endif
#if (FXAA_QUALITY_PRESET == 27)
    const float FxaaQualityP[10] = { 1, 1.5, 2, 2, 2, 2, 2, 2, 4, 8 };
#endif
#if (FXAA_QUALITY_PRESET == 28)
    const float FxaaQualityP[11] = { 1, 1.5, 2, 2, 2, 2, 2, 2, 2, 4, 8 };
#endif
#if (FXAA_QUALITY_PRESET == 29)
    const float FxaaQualityP[12] = { 1, 1.5, 2, 2, 2, 2, 2, 2, 2, 2, 4, 8 };
#endif
#if (FXAA_QUALITY_PRESET == 39)
    const float FxaaQualityP[12] = { 1, 1, 1, 1, 1, 1.5, 2, 2, 2, 2, 4, 8 };
#endif

bool fxaaStep(in sampler2D tex,
         in float lumaNN,
         inout vec2 posN, inout vec2 posP,
         inout bool doneN, inout float lumaEndN,
         inout bool doneP, inout float lumaEndP,
         in vec2 offNP,
         in float gradientScaled,
         in float q)
{
    if (!doneN)
    {
        lumaEndN = luminance(texture(tex, posN.xy)) - lumaNN * 0.5;
    }
    if (!doneP)
    {
        lumaEndP = luminance(texture(tex, posP.xy)) - lumaNN * 0.5;
    }
    doneN = abs(lumaEndN) >= gradientScaled;
    if (!doneN)
    {
        posN.x -= offNP.x * q;
        posN.y -= offNP.y * q;
    }
    doneP = abs(lumaEndP) >= gradientScaled;
    if (!doneP)
    {
        posP.x += offNP.x * q;
        posP.y += offNP.y * q;
    }
    return !doneN || !doneP;
}


vec3 fxaa(
    in vec2 posM,
    in sampler2D tex,
    in vec2 invScreenSizePixels,
    in float subpixelAA, // 0..1
    in float edgeThreshold, // 0.063..0.333
    in float edgeThresholdMin // 0.0312..0.0833
) {
/*--------------------------------------------------------------------------*/
    vec4 rgbyM = texture(tex, posM);
    float lumaM = luminance(rgbyM);
    float lumaS = luminance(textureOffset(tex, posM, ivec2(0, 1)));
    float lumaE = luminance(textureOffset(tex, posM, ivec2(1, 0)));
    float lumaN = luminance(textureOffset(tex, posM, ivec2(0,-1)));
    float lumaW = luminance(textureOffset(tex, posM, ivec2(-1, 0)));
/*--------------------------------------------------------------------------*/
    float maxSM = max(lumaS, lumaM);
    float minSM = min(lumaS, lumaM);
    float maxESM = max(lumaE, maxSM);
    float minESM = min(lumaE, minSM);
    float maxWN = max(lumaN, lumaW);
    float minWN = min(lumaN, lumaW);
    float rangeMax = max(maxWN, maxESM);
    float rangeMin = min(minWN, minESM);
    float rangeMaxScaled = rangeMax * edgeThreshold;
    float range = rangeMax - rangeMin;
    float rangeMaxClamped = max(edgeThresholdMin, rangeMaxScaled);
/*--------------------------------------------------------------------------*/
    if (range < rangeMaxClamped)
        return rgbyM.rgb;
/*--------------------------------------------------------------------------*/
    float lumaNW = luminance(textureOffset(tex, posM, ivec2(-1,-1)));
    float lumaSE = luminance(textureOffset(tex, posM, ivec2(1, 1)));
    float lumaNE = luminance(textureOffset(tex, posM, ivec2(1,-1)));
    float lumaSW = luminance(textureOffset(tex, posM, ivec2(-1, 1)));
/*--------------------------------------------------------------------------*/
    float lumaNS = lumaN + lumaS;
    float lumaWE = lumaW + lumaE;
    float subpixRcpRange = 1.0/range;
    float subpixNSWE = lumaNS + lumaWE;
    float edgeHorz1 = (-2.0 * lumaM) + lumaNS;
    float edgeVert1 = (-2.0 * lumaM) + lumaWE;
/*--------------------------------------------------------------------------*/
    float lumaNESE = lumaNE + lumaSE;
    float lumaNWNE = lumaNW + lumaNE;
    float edgeHorz2 = (-2.0 * lumaE) + lumaNESE;
    float edgeVert2 = (-2.0 * lumaN) + lumaNWNE;
/*--------------------------------------------------------------------------*/
    float lumaNWSW = lumaNW + lumaSW;
    float lumaSWSE = lumaSW + lumaSE;
    float edgeHorz4 = (abs(edgeHorz1) * 2.0) + abs(edgeHorz2);
    float edgeVert4 = (abs(edgeVert1) * 2.0) + abs(edgeVert2);
    float edgeHorz3 = (-2.0 * lumaW) + lumaNWSW;
    float edgeVert3 = (-2.0 * lumaS) + lumaSWSE;
    float edgeHorz = abs(edgeHorz3) + edgeHorz4;
    float edgeVert = abs(edgeVert3) + edgeVert4;
/*--------------------------------------------------------------------------*/
    float subpixNWSWNESE = lumaNWSW + lumaNESE;
    float lengthSign = invScreenSizePixels.x;
    bool horzSpan = edgeHorz >= edgeVert;
    float subpixA = subpixNSWE * 2.0 + subpixNWSWNESE;
/*--------------------------------------------------------------------------*/
    if (!horzSpan) lumaN = lumaW;
    if (!horzSpan) lumaS = lumaE;
    if (horzSpan) lengthSign = invScreenSizePixels.y;
    float subpixB = (subpixA * (1.0/12.0)) - lumaM;
/*--------------------------------------------------------------------------*/
    float gradientN = lumaN - lumaM;
    float gradientS = lumaS - lumaM;
    float lumaNN = lumaN + lumaM;
    float lumaSS = lumaS + lumaM;
    bool pairN = abs(gradientN) >= abs(gradientS);
    float gradient = max(abs(gradientN), abs(gradientS));
    if (pairN) lengthSign = -lengthSign;
    float subpixC = clamp(abs(subpixB) * subpixRcpRange, 0, 1);
/*--------------------------------------------------------------------------*/
    vec2 posB = posM;
    vec2 offNP;
    offNP.x = (!horzSpan) ? 0.0 : invScreenSizePixels.x;
    offNP.y = (horzSpan) ? 0.0 : invScreenSizePixels.y;
    if (!horzSpan) posB.x += lengthSign * 0.5;
    if (horzSpan) posB.y += lengthSign * 0.5;
/*--------------------------------------------------------------------------*/
    vec2 posN = posB - offNP * FxaaQualityP[0];
    vec2 posP = posB + offNP * FxaaQualityP[0];
    float lumaEndN = luminance(texture(tex, posN));
    float subpixE = subpixC * subpixC;
    float lumaEndP = luminance(texture(tex, posP));
/*--------------------------------------------------------------------------*/
    if (!pairN) lumaNN = lumaSS;
    float gradientScaled = gradient * 1.0/4.0;
    float lumaMM = lumaM - lumaNN * 0.5;
    float subpixF = -2 * subpixC + 3.0 * subpixE;
/*--------------------------------------------------------------------------*/
    bool doneN = false;
    bool doneP = false;
    for(int i=1; i<FxaaQualityP.length(); ++i)
    {
        if(!fxaaStep(tex, lumaNN, posN, posP, doneN, lumaEndN, doneP, lumaEndP, offNP, gradientScaled, FxaaQualityP[i]))
            break;
    }
/*--------------------------------------------------------------------------*/
    float dstN = posM.x - posN.x;
    float dstP = posP.x - posM.x;
    if (!horzSpan) dstN = posM.y - posN.y;
    if (!horzSpan) dstP = posP.y - posM.y;
/*--------------------------------------------------------------------------*/
    bool goodSpan = dstN < dstP
        ? (lumaEndN < 0.0) != (lumaMM < 0.0)
        : (lumaEndP < 0.0) != (lumaMM < 0.0);
    float pixelOffsetSubpix = max(
        goodSpan ? -min(dstN, dstP)/(dstP + dstN) + 0.5 : 0.0,
        subpixF * subpixF * subpixelAA
    );
    if (!horzSpan) posM.x += pixelOffsetSubpix * lengthSign;
    if (horzSpan) posM.y += pixelOffsetSubpix * lengthSign;
    return texture(tex, posM).rgb;
}
