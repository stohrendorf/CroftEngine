#pragma once
#include "soglb_core.hpp"
#include "glsc2_enabled_features.hpp"
namespace glsc2
{
// API feature levels
// #define API_LEVEL_GL_SC_VERSION_2_0

// API extensions
// #define WITH_API_EXTENSION_GL_EXT_texture_compression_s3tc
// #define WITH_API_EXTENSION_GL_OES_depth24
// #define WITH_API_EXTENSION_GL_OES_depth32
// #define WITH_API_EXTENSION_GL_OES_rgb8_rgba8
// #define WITH_API_EXTENSION_GL_OES_standard_derivatives

// enums
enum class TextureNormalModeEXT : core::EnumType
{
}; // enum TextureNormalModeEXT

enum class LightTexturePNameEXT : core::EnumType
{
}; // enum LightTexturePNameEXT

enum class VertexShaderCoordOutEXT : core::EnumType
{
}; // enum VertexShaderCoordOutEXT

enum class SamplePatternEXT : core::EnumType
{
}; // enum SamplePatternEXT

enum class VertexShaderStorageTypeEXT : core::EnumType
{
}; // enum VertexShaderStorageTypeEXT

enum class VertexShaderParameterEXT : core::EnumType
{
}; // enum VertexShaderParameterEXT

enum class LightTextureModeEXT : core::EnumType
{
}; // enum LightTextureModeEXT

enum class VertexShaderOpEXT : core::EnumType
{
}; // enum VertexShaderOpEXT

enum class ProgramFormatARB : core::EnumType
{
}; // enum ProgramFormatARB

enum class PointParameterNameARB : core::EnumType
{
}; // enum PointParameterNameARB

enum class VertexAttribPropertyARB : core::EnumType
{
}; // enum VertexAttribPropertyARB

enum class VertexAttribPointerPropertyARB : core::EnumType
{
}; // enum VertexAttribPointerPropertyARB

enum class ProgramStringPropertyARB : core::EnumType
{
}; // enum ProgramStringPropertyARB

enum class BufferPointerNameARB : core::EnumType
{
}; // enum BufferPointerNameARB

enum class BufferPNameARB : core::EnumType
{
}; // enum BufferPNameARB

enum class ClampColorModeARB : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    False = 0,
    True = 1,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum ClampColorModeARB

enum class ClampColorTargetARB : core::EnumType
{
}; // enum ClampColorTargetARB

enum class ProgramTargetARB : core::EnumType
{
}; // enum ProgramTargetARB

enum class VertexArrayPNameAPPLE : core::EnumType
{
}; // enum VertexArrayPNameAPPLE

enum class ObjectTypeAPPLE : core::EnumType
{
}; // enum ObjectTypeAPPLE

enum class PreserveModeATI : core::EnumType
{
}; // enum PreserveModeATI

enum class TexBumpParameterATI : core::EnumType
{
}; // enum TexBumpParameterATI

enum class SwizzleOpATI : core::EnumType
{
}; // enum SwizzleOpATI

enum class PNTrianglesPNameATI : core::EnumType
{
}; // enum PNTrianglesPNameATI

enum class ArrayObjectUsageATI : core::EnumType
{
}; // enum ArrayObjectUsageATI

enum class GetTexBumpParameterATI : core::EnumType
{
}; // enum GetTexBumpParameterATI

enum class ArrayObjectPNameATI : core::EnumType
{
}; // enum ArrayObjectPNameATI

enum class DrawBufferModeATI : core::EnumType
{
}; // enum DrawBufferModeATI

enum class VertexStreamATI : core::EnumType
{
}; // enum VertexStreamATI

enum class SpriteParameterNameSGIX : core::EnumType
{
}; // enum SpriteParameterNameSGIX

enum class PixelTexGenModeSGIX : core::EnumType
{
}; // enum PixelTexGenModeSGIX

enum class IglooFunctionSelectSGIX : core::EnumType
{
}; // enum IglooFunctionSelectSGIX

enum class HintTargetPGI : core::EnumType
{
}; // enum HintTargetPGI

enum class ImageTransformPNameHP : core::EnumType
{
}; // enum ImageTransformPNameHP

enum class ImageTransformTargetHP : core::EnumType
{
}; // enum ImageTransformTargetHP

enum class TextureFilterSGIS : core::EnumType
{
}; // enum TextureFilterSGIS

enum class OcclusionQueryParameterNameNV : core::EnumType
{
}; // enum OcclusionQueryParameterNameNV

enum class GetMultisamplePNameNV : core::EnumType
{
}; // enum GetMultisamplePNameNV

enum class MapParameterNV : core::EnumType
{
}; // enum MapParameterNV

enum class MapAttribParameterNV : core::EnumType
{
}; // enum MapAttribParameterNV

enum class FenceParameterNameNV : core::EnumType
{
}; // enum FenceParameterNameNV

enum class CombinerParameterNV : core::EnumType
{
}; // enum CombinerParameterNV

enum class CombinerBiasNV : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    None = 0,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum CombinerBiasNV

enum class CombinerScaleNV : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    None = 0,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum CombinerScaleNV

enum class CombinerMappingNV : core::EnumType
{
}; // enum CombinerMappingNV

enum class CombinerRegisterNV : core::EnumType
{
}; // enum CombinerRegisterNV

enum class CombinerVariableNV : core::EnumType
{
}; // enum CombinerVariableNV

enum class PixelDataRangeTargetNV : core::EnumType
{
}; // enum PixelDataRangeTargetNV

enum class EvalTargetNV : core::EnumType
{
}; // enum EvalTargetNV

enum class VertexAttribEnumNV : core::EnumType
{
}; // enum VertexAttribEnumNV

enum class FenceConditionNV : core::EnumType
{
}; // enum FenceConditionNV

enum class PathCoordType : core::EnumType
{
}; // enum PathCoordType

enum class AccumOp : core::EnumType
{
}; // enum AccumOp

enum class AttribMask : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    ColorBufferBit = 0x00004000,
    DepthBufferBit = 0x00000100,
    StencilBufferBit = 0x00000400,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum AttribMask
constexpr core::Bitfield<AttribMask> operator|(AttribMask left, AttribMask right) { return core::Bitfield<AttribMask>(left) | right;}

enum class AlphaFunction : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Always = 0x0207,
    Equal = 0x0202,
    Gequal = 0x0206,
    Greater = 0x0204,
    Lequal = 0x0203,
    Less = 0x0201,
    Never = 0x0200,
    Notequal = 0x0205,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum AlphaFunction

enum class BlendEquationModeEXT : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    FuncAdd = 0x8006,
    FuncReverseSubtract = 0x800B,
    FuncSubtract = 0x800A,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum BlendEquationModeEXT

enum class Boolean : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    False = 0,
    True = 1,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum Boolean

enum class BufferBitQCOM : core::EnumType
{
}; // enum BufferBitQCOM
constexpr core::Bitfield<BufferBitQCOM> operator|(BufferBitQCOM left, BufferBitQCOM right) { return core::Bitfield<BufferBitQCOM>(left) | right;}

enum class BufferTargetARB : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    ArrayBuffer = 0x8892,
    ElementArrayBuffer = 0x8893,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum BufferTargetARB

enum class BufferUsageARB : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    DynamicDraw = 0x88E8,
    StaticDraw = 0x88E4,
    StreamDraw = 0x88E0,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum BufferUsageARB

enum class BufferAccessARB : core::EnumType
{
}; // enum BufferAccessARB

enum class BufferStorageMask : core::EnumType
{
}; // enum BufferStorageMask
constexpr core::Bitfield<BufferStorageMask> operator|(BufferStorageMask left, BufferStorageMask right) { return core::Bitfield<BufferStorageMask>(left) | right;}

enum class ClearBufferMask : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    ColorBufferBit = 0x00004000,
    DepthBufferBit = 0x00000100,
    StencilBufferBit = 0x00000400,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum ClearBufferMask
constexpr core::Bitfield<ClearBufferMask> operator|(ClearBufferMask left, ClearBufferMask right) { return core::Bitfield<ClearBufferMask>(left) | right;}

enum class ClientAttribMask : core::EnumType
{
}; // enum ClientAttribMask
constexpr core::Bitfield<ClientAttribMask> operator|(ClientAttribMask left, ClientAttribMask right) { return core::Bitfield<ClientAttribMask>(left) | right;}

enum class ClipPlaneName : core::EnumType
{
}; // enum ClipPlaneName

enum class ColorMaterialFace : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Back = 0x0405,
    Front = 0x0404,
    FrontAndBack = 0x0408,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum ColorMaterialFace

enum class ColorMaterialParameter : core::EnumType
{
}; // enum ColorMaterialParameter

enum class ColorPointerType : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Byte = 0x1400,
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum ColorPointerType

enum class ColorTableParameterPNameSGI : core::EnumType
{
}; // enum ColorTableParameterPNameSGI

enum class ColorTableTargetSGI : core::EnumType
{
}; // enum ColorTableTargetSGI

enum class ContextFlagMask : core::EnumType
{
}; // enum ContextFlagMask
constexpr core::Bitfield<ContextFlagMask> operator|(ContextFlagMask left, ContextFlagMask right) { return core::Bitfield<ContextFlagMask>(left) | right;}

enum class ContextProfileMask : core::EnumType
{
}; // enum ContextProfileMask
constexpr core::Bitfield<ContextProfileMask> operator|(ContextProfileMask left, ContextProfileMask right) { return core::Bitfield<ContextProfileMask>(left) | right;}

enum class ConvolutionBorderModeEXT : core::EnumType
{
}; // enum ConvolutionBorderModeEXT

enum class ConvolutionParameterEXT : core::EnumType
{
}; // enum ConvolutionParameterEXT

enum class ConvolutionTargetEXT : core::EnumType
{
}; // enum ConvolutionTargetEXT

enum class CullFaceMode : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Back = 0x0405,
    Front = 0x0404,
    FrontAndBack = 0x0408,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum CullFaceMode

enum class DataType : core::EnumType
{
}; // enum DataType

enum class DepthFunction : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Always = 0x0207,
    Equal = 0x0202,
    Gequal = 0x0206,
    Greater = 0x0204,
    Lequal = 0x0203,
    Less = 0x0201,
    Never = 0x0200,
    Notequal = 0x0205,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum DepthFunction

enum class DrawBufferMode : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Back = 0x0405,
    ColorAttachment0 = 0x8CE0,
    Front = 0x0404,
    FrontAndBack = 0x0408,
    None = 0,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum DrawBufferMode

enum class DrawElementsType : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum DrawElementsType

enum class EnableCap : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Blend = 0x0BE2,
    CullFace = 0x0B44,
    DepthTest = 0x0B71,
    Dither = 0x0BD0,
    PolygonOffsetFill = 0x8037,
    SampleAlphaToCoverage = 0x809E,
    SampleCoverage = 0x80A0,
    ScissorTest = 0x0C11,
    StencilTest = 0x0B90,
    Texture2d = 0x0DE1,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum EnableCap

enum class ErrorCode : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    InvalidEnum = 0x0500,
    InvalidFramebufferOperation = 0x0506,
    InvalidOperation = 0x0502,
    InvalidValue = 0x0501,
    NoError = 0,
    OutOfMemory = 0x0505,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum ErrorCode

enum class ExternalHandleType : core::EnumType
{
}; // enum ExternalHandleType

enum class FeedbackType : core::EnumType
{
}; // enum FeedbackType

enum class FeedBackToken : core::EnumType
{
}; // enum FeedBackToken

enum class FfdMaskSGIX : core::EnumType
{
}; // enum FfdMaskSGIX
constexpr core::Bitfield<FfdMaskSGIX> operator|(FfdMaskSGIX left, FfdMaskSGIX right) { return core::Bitfield<FfdMaskSGIX>(left) | right;}

enum class FfdTargetSGIX : core::EnumType
{
}; // enum FfdTargetSGIX

enum class FogCoordinatePointerType : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Float = 0x1406,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum FogCoordinatePointerType

enum class FogMode : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Linear = 0x2601,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum FogMode

enum class FogParameter : core::EnumType
{
}; // enum FogParameter

enum class FogPointerTypeEXT : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Float = 0x1406,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum FogPointerTypeEXT

enum class FogPointerTypeIBM : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Float = 0x1406,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum FogPointerTypeIBM

enum class FragmentLightModelParameterSGIX : core::EnumType
{
}; // enum FragmentLightModelParameterSGIX

enum class FramebufferFetchNoncoherent : core::EnumType
{
}; // enum FramebufferFetchNoncoherent

enum class FrontFaceDirection : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Ccw = 0x0901,
    Cw = 0x0900,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum FrontFaceDirection

enum class GetColorTableParameterPNameSGI : core::EnumType
{
}; // enum GetColorTableParameterPNameSGI

enum class GetConvolutionParameter : core::EnumType
{
}; // enum GetConvolutionParameter

enum class GetHistogramParameterPNameEXT : core::EnumType
{
}; // enum GetHistogramParameterPNameEXT

enum class GetMapQuery : core::EnumType
{
}; // enum GetMapQuery

enum class GetMinmaxParameterPNameEXT : core::EnumType
{
}; // enum GetMinmaxParameterPNameEXT

enum class GetPixelMap : core::EnumType
{
}; // enum GetPixelMap

enum class GetPName : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    ActiveTexture = 0x84E0,
    AliasedLineWidthRange = 0x846E,
    AliasedPointSizeRange = 0x846D,
    AlphaBits = 0x0D55,
    ArrayBufferBinding = 0x8894,
    Blend = 0x0BE2,
    BlendColor = 0x8005,
    BlendDstAlpha = 0x80CA,
    BlendDstRgb = 0x80C8,
    BlendEquationAlpha = 0x883D,
    BlendEquationRgb = 0x8009,
    BlendSrcAlpha = 0x80CB,
    BlendSrcRgb = 0x80C9,
    BlueBits = 0x0D54,
    ColorClearValue = 0x0C22,
    ColorWritemask = 0x0C23,
    CompressedTextureFormats = 0x86A3,
    CullFace = 0x0B44,
    CullFaceMode = 0x0B45,
    CurrentProgram = 0x8B8D,
    DepthBits = 0x0D56,
    DepthClearValue = 0x0B73,
    DepthFunc = 0x0B74,
    DepthRange = 0x0B70,
    DepthTest = 0x0B71,
    DepthWritemask = 0x0B72,
    Dither = 0x0BD0,
    ElementArrayBufferBinding = 0x8895,
    FrontFace = 0x0B46,
    GreenBits = 0x0D53,
    ImplementationColorReadFormat = 0x8B9B,
    ImplementationColorReadType = 0x8B9A,
    LineWidth = 0x0B21,
    MaxCombinedTextureImageUnits = 0x8B4D,
    MaxFragmentUniformVectors = 0x8DFD,
    MaxRenderbufferSize = 0x84E8,
    MaxTextureImageUnits = 0x8872,
    MaxTextureSize = 0x0D33,
    MaxVaryingVectors = 0x8DFC,
    MaxVertexAttribs = 0x8869,
    MaxVertexTextureImageUnits = 0x8B4C,
    MaxVertexUniformVectors = 0x8DFB,
    MaxViewportDims = 0x0D3A,
    NumCompressedTextureFormats = 0x86A2,
    NumProgramBinaryFormats = 0x87FE,
    PackAlignment = 0x0D05,
    PolygonOffsetFactor = 0x8038,
    PolygonOffsetFill = 0x8037,
    PolygonOffsetUnits = 0x2A00,
    ProgramBinaryFormats = 0x87FF,
    RedBits = 0x0D52,
    RenderbufferBinding = 0x8CA7,
    Samples = 0x80A9,
    SampleBuffers = 0x80A8,
    SampleCoverageInvert = 0x80AB,
    SampleCoverageValue = 0x80AA,
    ScissorBox = 0x0C10,
    ScissorTest = 0x0C11,
    StencilBackFail = 0x8801,
    StencilBackFunc = 0x8800,
    StencilBackPassDepthFail = 0x8802,
    StencilBackPassDepthPass = 0x8803,
    StencilBackRef = 0x8CA3,
    StencilBackValueMask = 0x8CA4,
    StencilBackWritemask = 0x8CA5,
    StencilBits = 0x0D57,
    StencilClearValue = 0x0B91,
    StencilFail = 0x0B94,
    StencilFunc = 0x0B92,
    StencilPassDepthFail = 0x0B95,
    StencilPassDepthPass = 0x0B96,
    StencilRef = 0x0B97,
    StencilTest = 0x0B90,
    StencilValueMask = 0x0B93,
    StencilWritemask = 0x0B98,
    SubpixelBits = 0x0D50,
    Texture2d = 0x0DE1,
    TextureBinding2d = 0x8069,
    UnpackAlignment = 0x0CF5,
    Viewport = 0x0BA2,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum GetPName

enum class GetPointervPName : core::EnumType
{
}; // enum GetPointervPName

enum class GetTextureParameter : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    TextureMagFilter = 0x2800,
    TextureMinFilter = 0x2801,
    TextureWrapS = 0x2802,
    TextureWrapT = 0x2803,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum GetTextureParameter

enum class HintMode : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    DontCare = 0x1100,
    Fastest = 0x1101,
    Nicest = 0x1102,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum HintMode

enum class HintTarget : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    GenerateMipmapHint = 0x8192,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum HintTarget

enum class HistogramTargetEXT : core::EnumType
{
}; // enum HistogramTargetEXT

enum class IndexPointerType : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum IndexPointerType

enum class InterleavedArrayFormat : core::EnumType
{
}; // enum InterleavedArrayFormat

enum class LightEnvModeSGIX : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Replace = 0x1E01,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum LightEnvModeSGIX

enum class LightEnvParameterSGIX : core::EnumType
{
}; // enum LightEnvParameterSGIX

enum class LightModelColorControl : core::EnumType
{
}; // enum LightModelColorControl

enum class LightModelParameter : core::EnumType
{
}; // enum LightModelParameter

enum class LightName : core::EnumType
{
}; // enum LightName

enum class LightParameter : core::EnumType
{
}; // enum LightParameter

enum class ListMode : core::EnumType
{
}; // enum ListMode

enum class ListNameType : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Byte = 0x1400,
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum ListNameType

enum class ListParameterName : core::EnumType
{
}; // enum ListParameterName

enum class LogicOp : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Invert = 0x150A,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum LogicOp

enum class MapBufferAccessMask : core::EnumType
{
}; // enum MapBufferAccessMask
constexpr core::Bitfield<MapBufferAccessMask> operator|(MapBufferAccessMask left, MapBufferAccessMask right) { return core::Bitfield<MapBufferAccessMask>(left) | right;}

enum class MapTarget : core::EnumType
{
}; // enum MapTarget

enum class MapTextureFormatINTEL : core::EnumType
{
}; // enum MapTextureFormatINTEL

enum class MaterialFace : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Back = 0x0405,
    Front = 0x0404,
    FrontAndBack = 0x0408,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum MaterialFace

enum class MaterialParameter : core::EnumType
{
}; // enum MaterialParameter

enum class MatrixMode : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Texture = 0x1702,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum MatrixMode

enum class MemoryBarrierMask : core::EnumType
{
}; // enum MemoryBarrierMask
constexpr core::Bitfield<MemoryBarrierMask> operator|(MemoryBarrierMask left, MemoryBarrierMask right) { return core::Bitfield<MemoryBarrierMask>(left) | right;}

enum class MemoryObjectParameterName : core::EnumType
{
}; // enum MemoryObjectParameterName

enum class MeshMode1 : core::EnumType
{
}; // enum MeshMode1

enum class MeshMode2 : core::EnumType
{
}; // enum MeshMode2

enum class MinmaxTargetEXT : core::EnumType
{
}; // enum MinmaxTargetEXT

enum class NormalPointerType : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Byte = 0x1400,
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum NormalPointerType

enum class PixelCopyType : core::EnumType
{
}; // enum PixelCopyType

enum class PixelFormat : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Red = 0x1903,
    Rg = 0x8227,
    Rgb = 0x1907,
    Rgba = 0x1908,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum PixelFormat

enum class InternalFormat : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    DepthComponent16 = 0x81A5,
    R8 = 0x8229,
    Red = 0x1903,
    Rg = 0x8227,
    Rg8 = 0x822B,
    Rgb = 0x1907,
    Rgb5A1 = 0x8057,
    Rgb8 = 0x8051,
    Rgba = 0x1908,
    Rgba4 = 0x8056,
    Rgba8 = 0x8058,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum InternalFormat

enum class PixelMap : core::EnumType
{
}; // enum PixelMap

enum class PixelStoreParameter : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    PackAlignment = 0x0D05,
    UnpackAlignment = 0x0CF5,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum PixelStoreParameter

enum class PixelStoreResampleMode : core::EnumType
{
}; // enum PixelStoreResampleMode

enum class PixelStoreSubsampleRate : core::EnumType
{
}; // enum PixelStoreSubsampleRate

enum class PixelTexGenMode : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    None = 0,
    Rgb = 0x1907,
    Rgba = 0x1908,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum PixelTexGenMode

enum class PixelTexGenParameterNameSGIS : core::EnumType
{
}; // enum PixelTexGenParameterNameSGIS

enum class PixelTransferParameter : core::EnumType
{
}; // enum PixelTransferParameter

enum class PixelType : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Byte = 0x1400,
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
    UnsignedShort4444 = 0x8033,
    UnsignedShort5551 = 0x8034,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum PixelType

enum class PointParameterNameSGIS : core::EnumType
{
}; // enum PointParameterNameSGIS

enum class PolygonMode : core::EnumType
{
}; // enum PolygonMode

enum class PrimitiveType : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Lines = 0x0001,
    LineLoop = 0x0002,
    LineStrip = 0x0003,
    Points = 0x0000,
    Triangles = 0x0004,
    TriangleFan = 0x0006,
    TriangleStrip = 0x0005,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum PrimitiveType

enum class OcclusionQueryEventMaskAMD : core::EnumType
{
}; // enum OcclusionQueryEventMaskAMD
constexpr core::Bitfield<OcclusionQueryEventMaskAMD> operator|(OcclusionQueryEventMaskAMD left, OcclusionQueryEventMaskAMD right) { return core::Bitfield<OcclusionQueryEventMaskAMD>(left) | right;}

enum class ReadBufferMode : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Back = 0x0405,
    Front = 0x0404,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum ReadBufferMode

enum class RenderingMode : core::EnumType
{
}; // enum RenderingMode

enum class SamplePatternSGIS : core::EnumType
{
}; // enum SamplePatternSGIS

enum class SemaphoreParameterName : core::EnumType
{
}; // enum SemaphoreParameterName

enum class SeparableTargetEXT : core::EnumType
{
}; // enum SeparableTargetEXT

enum class ShadingModel : core::EnumType
{
}; // enum ShadingModel

enum class StencilFaceDirection : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Back = 0x0405,
    Front = 0x0404,
    FrontAndBack = 0x0408,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum StencilFaceDirection

enum class StencilFunction : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Always = 0x0207,
    Equal = 0x0202,
    Gequal = 0x0206,
    Greater = 0x0204,
    Lequal = 0x0203,
    Less = 0x0201,
    Never = 0x0200,
    Notequal = 0x0205,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum StencilFunction

enum class StencilOp : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Decr = 0x1E03,
    DecrWrap = 0x8508,
    Incr = 0x1E02,
    IncrWrap = 0x8507,
    Invert = 0x150A,
    Keep = 0x1E00,
    Replace = 0x1E01,
    Zero = 0,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum StencilOp

enum class StringName : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Extensions = 0x1F03,
    Renderer = 0x1F01,
    ShadingLanguageVersion = 0x8B8C,
    Vendor = 0x1F00,
    Version = 0x1F02,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum StringName

enum class SyncObjectMask : core::EnumType
{
}; // enum SyncObjectMask
constexpr core::Bitfield<SyncObjectMask> operator|(SyncObjectMask left, SyncObjectMask right) { return core::Bitfield<SyncObjectMask>(left) | right;}

enum class TexCoordPointerType : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum TexCoordPointerType

enum class TextureCoordName : core::EnumType
{
}; // enum TextureCoordName

enum class TextureEnvMode : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Blend = 0x0BE2,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum TextureEnvMode

enum class TextureEnvParameter : core::EnumType
{
}; // enum TextureEnvParameter

enum class TextureEnvTarget : core::EnumType
{
}; // enum TextureEnvTarget

enum class TextureFilterFuncSGIS : core::EnumType
{
}; // enum TextureFilterFuncSGIS

enum class TextureGenMode : core::EnumType
{
}; // enum TextureGenMode

enum class TextureGenParameter : core::EnumType
{
}; // enum TextureGenParameter

enum class TextureMagFilter : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Linear = 0x2601,
    Nearest = 0x2600,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum TextureMagFilter

enum class TextureMinFilter : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Linear = 0x2601,
    LinearMipmapLinear = 0x2703,
    LinearMipmapNearest = 0x2701,
    Nearest = 0x2600,
    NearestMipmapLinear = 0x2702,
    NearestMipmapNearest = 0x2700,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum TextureMinFilter

enum class TextureParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    TextureMagFilter = 0x2800,
    TextureMinFilter = 0x2801,
    TextureWrapS = 0x2802,
    TextureWrapT = 0x2803,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum TextureParameterName

enum class TextureStorageMaskAMD : core::EnumType
{
}; // enum TextureStorageMaskAMD
constexpr core::Bitfield<TextureStorageMaskAMD> operator|(TextureStorageMaskAMD left, TextureStorageMaskAMD right) { return core::Bitfield<TextureStorageMaskAMD>(left) | right;}

enum class TextureTarget : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Texture2d = 0x0DE1,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum TextureTarget

enum class TextureWrapMode : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    ClampToEdge = 0x812F,
    Repeat = 0x2901,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum TextureWrapMode

enum class UseProgramStageMask : core::EnumType
{
}; // enum UseProgramStageMask
constexpr core::Bitfield<UseProgramStageMask> operator|(UseProgramStageMask left, UseProgramStageMask right) { return core::Bitfield<UseProgramStageMask>(left) | right;}

enum class VertexPointerType : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum VertexPointerType

enum class FramebufferAttachment : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    ColorAttachment0 = 0x8CE0,
    DepthAttachment = 0x8D00,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum FramebufferAttachment

enum class RenderbufferTarget : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Renderbuffer = 0x8D41,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum RenderbufferTarget

enum class FramebufferTarget : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Framebuffer = 0x8D40,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum FramebufferTarget

enum class TextureUnit : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Texture0 = 0x84C0,
    Texture1 = 0x84C1,
    Texture10 = 0x84CA,
    Texture11 = 0x84CB,
    Texture12 = 0x84CC,
    Texture13 = 0x84CD,
    Texture14 = 0x84CE,
    Texture15 = 0x84CF,
    Texture16 = 0x84D0,
    Texture17 = 0x84D1,
    Texture18 = 0x84D2,
    Texture19 = 0x84D3,
    Texture2 = 0x84C2,
    Texture20 = 0x84D4,
    Texture21 = 0x84D5,
    Texture22 = 0x84D6,
    Texture23 = 0x84D7,
    Texture24 = 0x84D8,
    Texture25 = 0x84D9,
    Texture26 = 0x84DA,
    Texture27 = 0x84DB,
    Texture28 = 0x84DC,
    Texture29 = 0x84DD,
    Texture3 = 0x84C3,
    Texture30 = 0x84DE,
    Texture31 = 0x84DF,
    Texture4 = 0x84C4,
    Texture5 = 0x84C5,
    Texture6 = 0x84C6,
    Texture7 = 0x84C7,
    Texture8 = 0x84C8,
    Texture9 = 0x84C9,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum TextureUnit

enum class ConditionalRenderMode : core::EnumType
{
}; // enum ConditionalRenderMode

enum class FragmentOpATI : core::EnumType
{
}; // enum FragmentOpATI

enum class FramebufferStatus : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    FramebufferComplete = 0x8CD5,
    FramebufferIncompleteAttachment = 0x8CD6,
    FramebufferIncompleteMissingAttachment = 0x8CD7,
    FramebufferUndefined = 0x8219,
    FramebufferUnsupported = 0x8CDD,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum FramebufferStatus

enum class GraphicsResetStatus : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    GuiltyContextReset = 0x8253,
    InnocentContextReset = 0x8254,
    NoError = 0,
    UnknownContextReset = 0x8255,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum GraphicsResetStatus

enum class SyncStatus : core::EnumType
{
}; // enum SyncStatus

enum class QueryTarget : core::EnumType
{
}; // enum QueryTarget

enum class QueryCounterTarget : core::EnumType
{
}; // enum QueryCounterTarget

enum class ConvolutionTarget : core::EnumType
{
}; // enum ConvolutionTarget

enum class PathFillMode : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Invert = 0x150A,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum PathFillMode

enum class ColorTableTarget : core::EnumType
{
}; // enum ColorTableTarget

enum class VertexBufferObjectParameter : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    BufferSize = 0x8764,
    BufferUsage = 0x8765,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum VertexBufferObjectParameter

enum class RenderbufferParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    RenderbufferAlphaSize = 0x8D53,
    RenderbufferBlueSize = 0x8D52,
    RenderbufferDepthSize = 0x8D54,
    RenderbufferGreenSize = 0x8D51,
    RenderbufferHeight = 0x8D43,
    RenderbufferInternalFormat = 0x8D44,
    RenderbufferRedSize = 0x8D50,
    RenderbufferStencilSize = 0x8D55,
    RenderbufferWidth = 0x8D42,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum RenderbufferParameterName

enum class VertexBufferObjectUsage : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    DynamicDraw = 0x88E8,
    StaticDraw = 0x88E4,
    StreamDraw = 0x88E0,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum VertexBufferObjectUsage

enum class FramebufferParameterName : core::EnumType
{
}; // enum FramebufferParameterName

enum class ProgramParameterPName : core::EnumType
{
}; // enum ProgramParameterPName

enum class BlendingFactor : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    ConstantAlpha = 0x8003,
    ConstantColor = 0x8001,
    DstAlpha = 0x0304,
    DstColor = 0x0306,
    One = 1,
    OneMinusConstantAlpha = 0x8004,
    OneMinusConstantColor = 0x8002,
    OneMinusDstAlpha = 0x0305,
    OneMinusDstColor = 0x0307,
    OneMinusSrcAlpha = 0x0303,
    OneMinusSrcColor = 0x0301,
    SrcAlpha = 0x0302,
    SrcAlphaSaturate = 0x0308,
    SrcColor = 0x0300,
    Zero = 0,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum BlendingFactor

enum class BindTransformFeedbackTarget : core::EnumType
{
}; // enum BindTransformFeedbackTarget

enum class BlitFramebufferFilter : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Linear = 0x2601,
    Nearest = 0x2600,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum BlitFramebufferFilter

enum class BufferStorageTarget : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    ArrayBuffer = 0x8892,
    ElementArrayBuffer = 0x8893,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum BufferStorageTarget

enum class CheckFramebufferStatusTarget : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Framebuffer = 0x8D40,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum CheckFramebufferStatusTarget

enum class Buffer : core::EnumType
{
}; // enum Buffer

enum class ClipControlOrigin : core::EnumType
{
}; // enum ClipControlOrigin

enum class ClipControlDepth : core::EnumType
{
}; // enum ClipControlDepth

enum class CopyBufferSubDataTarget : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    ArrayBuffer = 0x8892,
    ElementArrayBuffer = 0x8893,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum CopyBufferSubDataTarget

enum class ShaderType : core::EnumType
{
}; // enum ShaderType

enum class DebugSource : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    DontCare = 0x1100,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum DebugSource

enum class DebugType : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    DontCare = 0x1100,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum DebugType

enum class DebugSeverity : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    DontCare = 0x1100,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum DebugSeverity

enum class SyncCondition : core::EnumType
{
}; // enum SyncCondition

enum class FogPName : core::EnumType
{
}; // enum FogPName

enum class AtomicCounterBufferPName : core::EnumType
{
}; // enum AtomicCounterBufferPName

enum class UniformBlockPName : core::EnumType
{
}; // enum UniformBlockPName

enum class UniformPName : core::EnumType
{
}; // enum UniformPName

enum class SamplerParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    TextureMagFilter = 0x2800,
    TextureMinFilter = 0x2801,
    TextureWrapS = 0x2802,
    TextureWrapT = 0x2803,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum SamplerParameterName

enum class VertexProvokingMode : core::EnumType
{
}; // enum VertexProvokingMode

enum class PatchParameterName : core::EnumType
{
}; // enum PatchParameterName

enum class ObjectIdentifier : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Framebuffer = 0x8D40,
    Renderbuffer = 0x8D41,
    Texture = 0x1702,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum ObjectIdentifier

enum class ColorBuffer : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Back = 0x0405,
    ColorAttachment0 = 0x8CE0,
    Front = 0x0404,
    FrontAndBack = 0x0408,
    None = 0,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum ColorBuffer

enum class MapQuery : core::EnumType
{
}; // enum MapQuery

enum class VertexArrayPName : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    VertexAttribArrayEnabled = 0x8622,
    VertexAttribArrayNormalized = 0x886A,
    VertexAttribArraySize = 0x8623,
    VertexAttribArrayStride = 0x8624,
    VertexAttribArrayType = 0x8625,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum VertexArrayPName

enum class TransformFeedbackPName : core::EnumType
{
}; // enum TransformFeedbackPName

enum class SyncParameterName : core::EnumType
{
}; // enum SyncParameterName

enum class ShaderParameterName : core::EnumType
{
}; // enum ShaderParameterName

enum class QueryObjectParameterName : core::EnumType
{
}; // enum QueryObjectParameterName

enum class QueryParameterName : core::EnumType
{
}; // enum QueryParameterName

enum class ProgramStagePName : core::EnumType
{
}; // enum ProgramStagePName

enum class PipelineParameterName : core::EnumType
{
}; // enum PipelineParameterName

enum class ProgramInterface : core::EnumType
{
}; // enum ProgramInterface

enum class VertexAttribEnum : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    CurrentVertexAttrib = 0x8626,
    VertexAttribArrayBufferBinding = 0x889F,
    VertexAttribArrayEnabled = 0x8622,
    VertexAttribArrayNormalized = 0x886A,
    VertexAttribArraySize = 0x8623,
    VertexAttribArrayStride = 0x8624,
    VertexAttribArrayType = 0x8625,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum VertexAttribEnum

enum class VertexAttribType : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Byte = 0x1400,
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum VertexAttribType

enum class AttributeType : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Sampler2d = 0x8B5E,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum AttributeType

enum class InternalFormatPName : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Samples = 0x80A9,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum InternalFormatPName

enum class FramebufferAttachmentParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    FramebufferAttachmentObjectName = 0x8CD1,
    FramebufferAttachmentTextureLevel = 0x8CD2,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum FramebufferAttachmentParameterName

enum class ProgramInterfacePName : core::EnumType
{
}; // enum ProgramInterfacePName

enum class PrecisionType : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    HighFloat = 0x8DF2,
    HighInt = 0x8DF5,
    LowFloat = 0x8DF0,
    LowInt = 0x8DF3,
    MediumFloat = 0x8DF1,
    MediumInt = 0x8DF4,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum PrecisionType

enum class VertexAttribPointerType : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    Byte = 0x1400,
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum VertexAttribPointerType

enum class SubroutineParameterName : core::EnumType
{
}; // enum SubroutineParameterName

enum class GetFramebufferParameter : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    ImplementationColorReadFormat = 0x8B9B,
    ImplementationColorReadType = 0x8B9A,
    Samples = 0x80A9,
    SampleBuffers = 0x80A8,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum GetFramebufferParameter

enum class PathStringFormat : core::EnumType
{
}; // enum PathStringFormat

enum class PathFontTarget : core::EnumType
{
}; // enum PathFontTarget

enum class PathHandleMissingGlyphs : core::EnumType
{
}; // enum PathHandleMissingGlyphs

enum class PathParameter : core::EnumType
{
}; // enum PathParameter

enum class PathColor : core::EnumType
{
}; // enum PathColor

enum class PathGenMode : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    None = 0,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum PathGenMode

enum class TextureLayout : core::EnumType
{
}; // enum TextureLayout

enum class PathTransformType : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    None = 0,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum PathTransformType

enum class PathElementType : core::EnumType
{
}; // enum PathElementType

enum class PathCoverMode : core::EnumType
{
}; // enum PathCoverMode

enum class PathFontStyle : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    None = 0,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum PathFontStyle

enum class PathMetricMask : core::EnumType
{
}; // enum PathMetricMask

enum class PathListMode : core::EnumType
{
}; // enum PathListMode

enum class ProgramPropertyARB : core::EnumType
{
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
    LinkStatus = 0x8B82,
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)
}; // enum ProgramPropertyARB

// commands
#if defined(API_LEVEL_GL_SC_VERSION_2_0)
extern void blendEquationSeparate(BlendEquationModeEXT modeRGB, BlendEquationModeEXT modeAlpha);
extern void bindRenderbuffer(RenderbufferTarget target, uint32_t renderbuffer);
extern void bindBuffer(BufferTargetARB target, uint32_t buffer);
extern void uniform1(int32_t location, core::SizeType count, const float *value);
extern void uniform2(int32_t location, int32_t v0, int32_t v1);
extern void sampleCoverage(float value, bool invert);
extern void enable(EnableCap cap);
extern void genRenderbuffers(core::SizeType n, uint32_t *renderbuffers);
extern void clearStencil(int32_t s);
extern void getTexParameter(TextureTarget target, GetTextureParameter pname, int32_t *params);
extern void blendEquation(BlendEquationModeEXT mode);
extern int32_t getAttribLocation(uint32_t program, const char *name);
extern void getTexParameter(TextureTarget target, GetTextureParameter pname, float *params);
extern void depthMask(bool flag);
extern core::EnumType checkFramebufferStatus(FramebufferTarget target);
extern void framebufferRenderbuffer(FramebufferTarget target, FramebufferAttachment attachment, RenderbufferTarget renderbuffertarget, uint32_t renderbuffer);
extern void hint(HintTarget target, HintMode mode);
extern void getVertexAttrib(uint32_t index, VertexAttribPropertyARB pname, float *params);
extern void uniform3(int32_t location, core::SizeType count, const float *value);
extern void uniform2(int32_t location, core::SizeType count, const float *value);
extern void uniform3(int32_t location, int32_t v0, int32_t v1, int32_t v2);
extern void enableVertexAttribArray(uint32_t index);
extern void readnPixel(int32_t x, int32_t y, core::SizeType width, core::SizeType height, PixelFormat format, PixelType type, core::SizeType bufSize, void *data);
extern void uniform4(int32_t location, core::SizeType count, const int32_t *value);
extern void programBinary(uint32_t program, core::EnumType binaryFormat, const void *binary, core::SizeType length);
extern void clearDepth(float d);
extern void blendFuncSeparate(BlendingFactor sfactorRGB, BlendingFactor dfactorRGB, BlendingFactor sfactorAlpha, BlendingFactor dfactorAlpha);
extern void texSubImage2D(TextureTarget target, int32_t level, int32_t xoffset, int32_t yoffset, core::SizeType width, core::SizeType height, PixelFormat format, PixelType type, const void *pixels);
extern void uniform2(int32_t location, float v0, float v1);
extern uint32_t createProgram();
extern void scissor(int32_t x, int32_t y, core::SizeType width, core::SizeType height);
extern void blendFunc(BlendingFactor sfactor, BlendingFactor dfactor);
extern void flush();
extern void getBooleanv(GetPName pname, bool *data);
extern void stencilFunc(StencilFunction func, int32_t ref, uint32_t mask);
extern void finish();
extern void texParameter(TextureTarget target, TextureParameterName pname, int32_t param);
extern void uniform1(int32_t location, int32_t v0);
extern void bufferData(BufferTargetARB target, std::size_t size, const void *data, BufferUsageARB usage);
extern void bufferSubData(BufferTargetARB target, std::intptr_t offset, std::size_t size, const void *data);
extern core::EnumType getGraphicsResetStatus();
extern void vertexAttrib4(uint32_t index, float x, float y, float z, float w);
extern void texStorage2D(TextureTarget target, core::SizeType levels, InternalFormat internalformat, core::SizeType width, core::SizeType height);
extern void genTextures(core::SizeType n, uint32_t *textures);
extern void vertexAttrib3(uint32_t index, const float *v);
extern void uniform4(int32_t location, float v0, float v1, float v2, float v3);
extern core::EnumType getError();
extern void renderbufferStorage(RenderbufferTarget target, InternalFormat internalformat, core::SizeType width, core::SizeType height);
extern void texParameter(TextureTarget target, TextureParameterName pname, const int32_t *params);
extern void drawRangeElements(PrimitiveType mode, uint32_t start, uint32_t end, core::SizeType count, DrawElementsType type, const void *indices);
extern void frontFace(FrontFaceDirection mode);
extern void getIntegerv(GetPName pname, int32_t *data);
extern void colorMask(bool red, bool green, bool blue, bool alpha);
extern void getVertexAttrib(uint32_t index, VertexAttribPropertyARB pname, int32_t *params);
extern void getnUniform(uint32_t program, int32_t location, core::SizeType bufSize, int32_t *params);
extern void stencilFuncSeparate(StencilFaceDirection face, StencilFunction func, int32_t ref, uint32_t mask);
extern void polygonOffset(float factor, float units);
extern void uniform1(int32_t location, core::SizeType count, const int32_t *value);
extern void getVertexAttribPointerv(uint32_t index, VertexAttribPointerPropertyARB pname, void **pointer);
extern void uniform3(int32_t location, float v0, float v1, float v2);
extern int32_t getUniformLocation(uint32_t program, const char *name);
extern void genBuffers(core::SizeType n, uint32_t *buffers);
extern void stencilMaskSeparate(StencilFaceDirection face, uint32_t mask);
extern void cullFace(CullFaceMode mode);
extern void disable(EnableCap cap);
extern void getProgram(uint32_t program, ProgramPropertyARB pname, int32_t *params);
extern void clear(core::Bitfield<ClearBufferMask> mask);
extern void vertexAttrib1(uint32_t index, float x);
extern void bindTexture(TextureTarget target, uint32_t texture);
extern void uniformMatrix4(int32_t location, core::SizeType count, bool transpose, const float *value);
extern void vertexAttrib4(uint32_t index, const float *v);
extern void vertexAttribPointer(uint32_t index, int32_t size, VertexAttribPointerType type, bool normalized, core::SizeType stride, const void *pointer);
extern void uniform3(int32_t location, core::SizeType count, const int32_t *value);
extern void activeTexture(TextureUnit texture);
extern void bindFramebuffer(FramebufferTarget target, uint32_t framebuffer);
extern void uniformMatrix2(int32_t location, core::SizeType count, bool transpose, const float *value);
extern void stencilOpSeparate(StencilFaceDirection face, StencilOp sfail, StencilOp dpfail, StencilOp dppass);
extern const uint8_t *getString(StringName name);
extern void uniformMatrix3(int32_t location, core::SizeType count, bool transpose, const float *value);
extern void vertexAttrib1(uint32_t index, const float *v);
extern void getFramebufferAttachmentParameter(FramebufferTarget target, FramebufferAttachment attachment, FramebufferAttachmentParameterName pname, int32_t *params);
extern void lineWidth(float width);
extern void uniform2(int32_t location, core::SizeType count, const int32_t *value);
extern void vertexAttrib3(uint32_t index, float x, float y, float z);
extern void stencilMask(uint32_t mask);
extern void clearColor(float red, float green, float blue, float alpha);
extern void stencilOp(StencilOp fail, StencilOp zfail, StencilOp zpass);
extern void texParameter(TextureTarget target, TextureParameterName pname, float param);
extern void uniform4(int32_t location, int32_t v0, int32_t v1, int32_t v2, int32_t v3);
extern void uniform4(int32_t location, core::SizeType count, const float *value);
extern void drawArrays(PrimitiveType mode, int32_t first, core::SizeType count);
extern bool isEnable(EnableCap cap);
extern void depthFunc(DepthFunction func);
extern void genFramebuffers(core::SizeType n, uint32_t *framebuffers);
extern void useProgram(uint32_t program);
extern void compressedTexSubImage2D(TextureTarget target, int32_t level, int32_t xoffset, int32_t yoffset, core::SizeType width, core::SizeType height, PixelFormat format, core::SizeType imageSize, const void *data);
extern void disableVertexAttribArray(uint32_t index);
extern void framebufferTexture2D(FramebufferTarget target, FramebufferAttachment attachment, TextureTarget textarget, uint32_t texture, int32_t level);
extern void viewport(int32_t x, int32_t y, core::SizeType width, core::SizeType height);
extern void texParameter(TextureTarget target, TextureParameterName pname, const float *params);
extern void getnUniform(uint32_t program, int32_t location, core::SizeType bufSize, float *params);
extern void vertexAttrib2(uint32_t index, float x, float y);
extern void generateMipmap(TextureTarget target);
extern void pixelStore(PixelStoreParameter pname, int32_t param);
extern void blendColor(float red, float green, float blue, float alpha);
extern void getFloatv(GetPName pname, float *data);
extern void uniform1(int32_t location, float v0);
extern void vertexAttrib2(uint32_t index, const float *v);
extern void depthRange(float n, float f);
extern void getRenderbufferParameter(RenderbufferTarget target, RenderbufferParameterName pname, int32_t *params);
extern void getBufferParameter(BufferTargetARB target, BufferPNameARB pname, int32_t *params);
#endif // defined(API_LEVEL_GL_SC_VERSION_2_0)

}
