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

bool fxaaStep(
in sampler2D tex,
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
in sampler2D tex,
in vec2 texCoordM,
in float subpixelAA, // 0..1
in float edgeThreshold, // 0.063..0.333
in float edgeThresholdMin// 0.0312..0.0833
) {
    vec2 posM = texCoordM;
    vec3 rgbM = texture(tex, posM).rgb;
    float lumaM = luminance(rgbM);
    float lumaS = luminance(textureOffset(tex, posM, ivec2(0, 1)));
    float lumaE = luminance(textureOffset(tex, posM, ivec2(1, 0)));
    float lumaN = luminance(textureOffset(tex, posM, ivec2(0, -1)));
    float lumaW = luminance(textureOffset(tex, posM, ivec2(-1, 0)));

    float lumaMax = max(max(lumaN, lumaW), max(lumaE, max(lumaS, lumaM)));
    float lumaMin = min(min(lumaN, lumaW), min(lumaE, min(lumaS, lumaM)));
    float lumaRange = lumaMax - lumaMin;
    if (lumaRange < max(edgeThresholdMin, lumaMax * edgeThreshold))
    return rgbM;

    float lumaNW = luminance(textureOffset(tex, posM, ivec2(-1, -1)));
    float lumaSE = luminance(textureOffset(tex, posM, ivec2(1, 1)));
    float lumaNE = luminance(textureOffset(tex, posM, ivec2(1, -1)));
    float lumaSW = luminance(textureOffset(tex, posM, ivec2(-1, 1)));
    /*--------------------------------------------------------------------------*/
    float lumaV = lumaN + lumaS;
    float lumaH = lumaW + lumaE;
    float lumaRight = lumaNE + lumaSE;
    float lumaTop = lumaNW + lumaNE;
    float lumaLeft = lumaNW + lumaSW;
    float lumaBottom = lumaSW + lumaSE;
    float edgeHorz = abs(-2.0 * lumaW + lumaLeft) + abs(-2.0 * lumaM + lumaV) * 2.0 + abs(-2.0 * lumaE + lumaRight);
    float edgeVert = abs(-2.0 * lumaS + lumaBottom) + abs(-2.0 * lumaM + lumaH) * 2.0 + abs(-2.0 * lumaN + lumaTop);
    /*--------------------------------------------------------------------------*/
    float lengthSign = 1.0/inputSize.x;
    bool horzSpan = edgeHorz >= edgeVert;
    float subpixA = (lumaV + lumaH) * 2.0 + lumaLeft + lumaRight;
    /*--------------------------------------------------------------------------*/
    if (!horzSpan) {
        lumaN = lumaW;
        lumaS = lumaE;
    }
    else {
        lengthSign = 1.0/inputSize.y;
    }
    /*--------------------------------------------------------------------------*/
    float gradientN = abs(lumaN - lumaM);
    float gradientS = abs(lumaS - lumaM);
    float lumaNN = lumaN + lumaM;
    float lumaSS = lumaS + lumaM;
    bool pairN = gradientN >= gradientS;
    if (pairN) {
        lengthSign = -lengthSign;
    }
    float subpixC = clamp(abs(subpixA / 12.0 - lumaM) / lumaRange, 0, 1);
    /*--------------------------------------------------------------------------*/
    vec2 posB = posM;
    vec2 offNP = horzSpan
    ? vec2(1.0/inputSize.x, 0.0)
    : vec2(0.0, 1.0/inputSize.y);
    if (!horzSpan) {
        posB.x += lengthSign * 0.5;
    }
    else {
        posB.y += lengthSign * 0.5;
    }
    /*--------------------------------------------------------------------------*/
    vec2 posN = posB - offNP * FxaaQualityP[0];
    vec2 posP = posB + offNP * FxaaQualityP[0];
    float lumaEndN = luminance(texture(tex, posN));
    float lumaEndP = luminance(texture(tex, posP));
    /*--------------------------------------------------------------------------*/
    if (!pairN) {
        lumaNN = lumaSS;
    }
    float gradientScaled = max(gradientN, gradientS) / 4;
    float lumaMM = lumaM - lumaNN * 0.5;
    float subpixF = -2 * subpixC + 3.0 * subpixC * subpixC;
    /*--------------------------------------------------------------------------*/
    bool doneN = false;
    bool doneP = false;
    for (int i=1; i<FxaaQualityP.length(); ++i)
    {
        if (!fxaaStep(tex, lumaNN, posN, posP, doneN, lumaEndN, doneP, lumaEndP, offNP, gradientScaled, FxaaQualityP[i]))
        break;
    }
    /*--------------------------------------------------------------------------*/
    float dstN = posM.x - posN.x;
    float dstP = posP.x - posM.x;
    if (!horzSpan) {
        dstN = posM.y - posN.y;
        dstP = posP.y - posM.y;
    }
    /*--------------------------------------------------------------------------*/
    bool goodSpan = dstN < dstP
    ? (lumaEndN < 0.0) != (lumaMM < 0.0)
    : (lumaEndP < 0.0) != (lumaMM < 0.0);
    float pixelOffsetSubpix = max(
    goodSpan ? -min(dstN, dstP)/(dstP + dstN) + 0.5 : 0.0,
    subpixF * subpixF * subpixelAA
    );
    if (!horzSpan) {
        posM.x += pixelOffsetSubpix * lengthSign;
    }
    else {
        posM.y += pixelOffsetSubpix * lengthSign;
    }
    return texture(tex, posM).rgb;
}
