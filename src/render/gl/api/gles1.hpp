#pragma once
#include "soglb_core.hpp"
#include "gles1_enabled_features.hpp"
namespace gles1
{
// API feature levels
// #define API_LEVEL_GL_VERSION_ES_CM_1_0_common

// API extensions
// #define WITH_API_EXTENSION_GL_AMD_compressed_3DC_texture
// #define WITH_API_EXTENSION_GL_AMD_compressed_ATC_texture
// #define WITH_API_EXTENSION_GL_APPLE_copy_texture_levels
// #define WITH_API_EXTENSION_GL_APPLE_framebuffer_multisample
// #define WITH_API_EXTENSION_GL_APPLE_sync
// #define WITH_API_EXTENSION_GL_APPLE_texture_format_BGRA8888
// #define WITH_API_EXTENSION_GL_APPLE_texture_max_level
// #define WITH_API_EXTENSION_GL_EXT_blend_minmax
// #define WITH_API_EXTENSION_GL_EXT_debug_marker
// #define WITH_API_EXTENSION_GL_EXT_discard_framebuffer
// #define WITH_API_EXTENSION_GL_EXT_map_buffer_range
// #define WITH_API_EXTENSION_GL_EXT_multi_draw_arrays
// #define WITH_API_EXTENSION_GL_EXT_multisampled_render_to_texture
// #define WITH_API_EXTENSION_GL_EXT_read_format_bgra
// #define WITH_API_EXTENSION_GL_EXT_robustness
// #define WITH_API_EXTENSION_GL_EXT_sRGB
// #define WITH_API_EXTENSION_GL_EXT_texture_compression_dxt1
// #define WITH_API_EXTENSION_GL_EXT_texture_filter_anisotropic
// #define WITH_API_EXTENSION_GL_EXT_texture_format_BGRA8888
// #define WITH_API_EXTENSION_GL_EXT_texture_lod_bias
// #define WITH_API_EXTENSION_GL_EXT_texture_storage
// #define WITH_API_EXTENSION_GL_IMG_multisampled_render_to_texture
// #define WITH_API_EXTENSION_GL_IMG_read_format
// #define WITH_API_EXTENSION_GL_IMG_texture_compression_pvrtc
// #define WITH_API_EXTENSION_GL_IMG_texture_env_enhanced_fixed_function
// #define WITH_API_EXTENSION_GL_IMG_user_clip_plane
// #define WITH_API_EXTENSION_GL_NV_fence
// #define WITH_API_EXTENSION_GL_OES_EGL_image
// #define WITH_API_EXTENSION_GL_OES_EGL_image_external
// #define WITH_API_EXTENSION_GL_OES_blend_equation_separate
// #define WITH_API_EXTENSION_GL_OES_blend_func_separate
// #define WITH_API_EXTENSION_GL_OES_blend_subtract
// #define WITH_API_EXTENSION_GL_OES_byte_coordinates
// #define WITH_API_EXTENSION_GL_OES_compressed_ETC1_RGB8_texture
// #define WITH_API_EXTENSION_GL_OES_compressed_paletted_texture
// #define WITH_API_EXTENSION_GL_OES_depth24
// #define WITH_API_EXTENSION_GL_OES_depth32
// #define WITH_API_EXTENSION_GL_OES_draw_texture
// #define WITH_API_EXTENSION_GL_OES_element_index_uint
// #define WITH_API_EXTENSION_GL_OES_fixed_point
// #define WITH_API_EXTENSION_GL_OES_framebuffer_object
// #define WITH_API_EXTENSION_GL_OES_mapbuffer
// #define WITH_API_EXTENSION_GL_OES_matrix_get
// #define WITH_API_EXTENSION_GL_OES_matrix_palette
// #define WITH_API_EXTENSION_GL_OES_packed_depth_stencil
// #define WITH_API_EXTENSION_GL_OES_point_size_array
// #define WITH_API_EXTENSION_GL_OES_point_sprite
// #define WITH_API_EXTENSION_GL_OES_query_matrix
// #define WITH_API_EXTENSION_GL_OES_read_format
// #define WITH_API_EXTENSION_GL_OES_required_internalformat
// #define WITH_API_EXTENSION_GL_OES_rgb8_rgba8
// #define WITH_API_EXTENSION_GL_OES_single_precision
// #define WITH_API_EXTENSION_GL_OES_stencil1
// #define WITH_API_EXTENSION_GL_OES_stencil4
// #define WITH_API_EXTENSION_GL_OES_stencil8
// #define WITH_API_EXTENSION_GL_OES_stencil_wrap
// #define WITH_API_EXTENSION_GL_OES_surfaceless_context
// #define WITH_API_EXTENSION_GL_OES_texture_cube_map
// #define WITH_API_EXTENSION_GL_OES_texture_mirrored_repeat
// #define WITH_API_EXTENSION_GL_OES_vertex_array_object
// #define WITH_API_EXTENSION_GL_QCOM_driver_control
// #define WITH_API_EXTENSION_GL_QCOM_extended_get
// #define WITH_API_EXTENSION_GL_QCOM_extended_get2
// #define WITH_API_EXTENSION_GL_QCOM_perfmon_global_mode
// #define WITH_API_EXTENSION_GL_QCOM_tiled_rendering
// #define WITH_API_EXTENSION_GL_QCOM_writeonly_rendering

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
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    False = 0,
    True = 1,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
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
}; // enum CombinerBiasNV

enum class CombinerScaleNV : core::EnumType
{
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
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Add = 0x0104,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum AccumOp

enum class AttribMask : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    ColorBufferBit = 0x00004000,
    DepthBufferBit = 0x00000100,
    StencilBufferBit = 0x00000400,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum AttribMask
constexpr core::Bitfield<AttribMask> operator|(AttribMask left, AttribMask right) { return core::Bitfield<AttribMask>(left) | right;}

enum class AlphaFunction : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Always = 0x0207,
    Equal = 0x0202,
    Gequal = 0x0206,
    Greater = 0x0204,
    Lequal = 0x0203,
    Less = 0x0201,
    Never = 0x0200,
    Notequal = 0x0205,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum AlphaFunction

enum class BlendEquationModeEXT : core::EnumType
{
}; // enum BlendEquationModeEXT

enum class Boolean : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    False = 0,
    True = 1,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum Boolean

enum class BufferBitQCOM : core::EnumType
{
}; // enum BufferBitQCOM
constexpr core::Bitfield<BufferBitQCOM> operator|(BufferBitQCOM left, BufferBitQCOM right) { return core::Bitfield<BufferBitQCOM>(left) | right;}

enum class BufferTargetARB : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    ArrayBuffer = 0x8892,
    ElementArrayBuffer = 0x8893,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum BufferTargetARB

enum class BufferUsageARB : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    DynamicDraw = 0x88E8,
    StaticDraw = 0x88E4,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
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
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    ColorBufferBit = 0x00004000,
    DepthBufferBit = 0x00000100,
    StencilBufferBit = 0x00000400,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum ClearBufferMask
constexpr core::Bitfield<ClearBufferMask> operator|(ClearBufferMask left, ClearBufferMask right) { return core::Bitfield<ClearBufferMask>(left) | right;}

enum class ClientAttribMask : core::EnumType
{
}; // enum ClientAttribMask
constexpr core::Bitfield<ClientAttribMask> operator|(ClientAttribMask left, ClientAttribMask right) { return core::Bitfield<ClientAttribMask>(left) | right;}

enum class ClipPlaneName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    ClipPlane0 = 0x3000,
    ClipPlane1 = 0x3001,
    ClipPlane2 = 0x3002,
    ClipPlane3 = 0x3003,
    ClipPlane4 = 0x3004,
    ClipPlane5 = 0x3005,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum ClipPlaneName

enum class ColorMaterialFace : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Back = 0x0405,
    Front = 0x0404,
    FrontAndBack = 0x0408,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum ColorMaterialFace

enum class ColorMaterialParameter : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Ambient = 0x1200,
    AmbientAndDiffuse = 0x1602,
    Diffuse = 0x1201,
    Emission = 0x1600,
    Specular = 0x1202,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum ColorMaterialParameter

enum class ColorPointerType : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Byte = 0x1400,
    Float = 0x1406,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedShort = 0x1403,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
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
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Back = 0x0405,
    Front = 0x0404,
    FrontAndBack = 0x0408,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum CullFaceMode

enum class DataType : core::EnumType
{
}; // enum DataType

enum class DepthFunction : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Always = 0x0207,
    Equal = 0x0202,
    Gequal = 0x0206,
    Greater = 0x0204,
    Lequal = 0x0203,
    Less = 0x0201,
    Never = 0x0200,
    Notequal = 0x0205,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum DepthFunction

enum class DrawBufferMode : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Back = 0x0405,
    Front = 0x0404,
    FrontAndBack = 0x0408,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum DrawBufferMode

enum class DrawElementsType : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    UnsignedByte = 0x1401,
    UnsignedShort = 0x1403,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum DrawElementsType

enum class EnableCap : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    AlphaTest = 0x0BC0,
    Blend = 0x0BE2,
    ClipPlane0 = 0x3000,
    ClipPlane1 = 0x3001,
    ClipPlane2 = 0x3002,
    ClipPlane3 = 0x3003,
    ClipPlane4 = 0x3004,
    ClipPlane5 = 0x3005,
    ColorArray = 0x8076,
    ColorLogicOp = 0x0BF2,
    ColorMaterial = 0x0B57,
    CullFace = 0x0B44,
    DepthTest = 0x0B71,
    Dither = 0x0BD0,
    Fog = 0x0B60,
    Light0 = 0x4000,
    Light1 = 0x4001,
    Light2 = 0x4002,
    Light3 = 0x4003,
    Light4 = 0x4004,
    Light5 = 0x4005,
    Light6 = 0x4006,
    Light7 = 0x4007,
    Lighting = 0x0B50,
    LineSmooth = 0x0B20,
    Multisample = 0x809D,
    Normalize = 0x0BA1,
    NormalArray = 0x8075,
    PointSmooth = 0x0B10,
    PolygonOffsetFill = 0x8037,
    SampleAlphaToCoverage = 0x809E,
    SampleAlphaToOne = 0x809F,
    SampleCoverage = 0x80A0,
    ScissorTest = 0x0C11,
    StencilTest = 0x0B90,
    Texture2d = 0x0DE1,
    TextureCoordArray = 0x8078,
    VertexArray = 0x8074,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum EnableCap

enum class ErrorCode : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    InvalidEnum = 0x0500,
    InvalidOperation = 0x0502,
    InvalidValue = 0x0501,
    NoError = 0,
    OutOfMemory = 0x0505,
    StackOverflow = 0x0503,
    StackUnderflow = 0x0504,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
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
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Float = 0x1406,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum FogCoordinatePointerType

enum class FogMode : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Exp = 0x0800,
    Exp2 = 0x0801,
    Linear = 0x2601,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum FogMode

enum class FogParameter : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    FogColor = 0x0B66,
    FogDensity = 0x0B62,
    FogEnd = 0x0B64,
    FogMode = 0x0B65,
    FogStart = 0x0B63,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum FogParameter

enum class FogPointerTypeEXT : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Float = 0x1406,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum FogPointerTypeEXT

enum class FogPointerTypeIBM : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Float = 0x1406,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum FogPointerTypeIBM

enum class FragmentLightModelParameterSGIX : core::EnumType
{
}; // enum FragmentLightModelParameterSGIX

enum class FramebufferFetchNoncoherent : core::EnumType
{
}; // enum FramebufferFetchNoncoherent

enum class FrontFaceDirection : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Ccw = 0x0901,
    Cw = 0x0900,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
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
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    ActiveTexture = 0x84E0,
    AliasedLineWidthRange = 0x846E,
    AliasedPointSizeRange = 0x846D,
    AlphaBits = 0x0D55,
    AlphaScale = 0x0D1C,
    AlphaTest = 0x0BC0,
    AlphaTestFunc = 0x0BC1,
    AlphaTestRef = 0x0BC2,
    ArrayBufferBinding = 0x8894,
    Blend = 0x0BE2,
    BlendDst = 0x0BE0,
    BlendSrc = 0x0BE1,
    BlueBits = 0x0D54,
    ClipPlane0 = 0x3000,
    ClipPlane1 = 0x3001,
    ClipPlane2 = 0x3002,
    ClipPlane3 = 0x3003,
    ClipPlane4 = 0x3004,
    ClipPlane5 = 0x3005,
    ColorArray = 0x8076,
    ColorArraySize = 0x8081,
    ColorArrayStride = 0x8083,
    ColorArrayType = 0x8082,
    ColorClearValue = 0x0C22,
    ColorLogicOp = 0x0BF2,
    ColorMaterial = 0x0B57,
    ColorWritemask = 0x0C23,
    CompressedTextureFormats = 0x86A3,
    CullFace = 0x0B44,
    CullFaceMode = 0x0B45,
    CurrentColor = 0x0B00,
    CurrentNormal = 0x0B02,
    CurrentTextureCoords = 0x0B03,
    DepthBits = 0x0D56,
    DepthClearValue = 0x0B73,
    DepthFunc = 0x0B74,
    DepthRange = 0x0B70,
    DepthTest = 0x0B71,
    DepthWritemask = 0x0B72,
    Dither = 0x0BD0,
    ElementArrayBufferBinding = 0x8895,
    Fog = 0x0B60,
    FogColor = 0x0B66,
    FogDensity = 0x0B62,
    FogEnd = 0x0B64,
    FogHint = 0x0C54,
    FogMode = 0x0B65,
    FogStart = 0x0B63,
    FrontFace = 0x0B46,
    GreenBits = 0x0D53,
    Light0 = 0x4000,
    Light1 = 0x4001,
    Light2 = 0x4002,
    Light3 = 0x4003,
    Light4 = 0x4004,
    Light5 = 0x4005,
    Light6 = 0x4006,
    Light7 = 0x4007,
    Lighting = 0x0B50,
    LightModelAmbient = 0x0B53,
    LightModelTwoSide = 0x0B52,
    LineSmooth = 0x0B20,
    LineSmoothHint = 0x0C52,
    LineWidth = 0x0B21,
    LogicOpMode = 0x0BF0,
    MatrixMode = 0x0BA0,
    MaxClipPlanes = 0x0D32,
    MaxLights = 0x0D31,
    MaxModelviewStackDepth = 0x0D36,
    MaxProjectionStackDepth = 0x0D38,
    MaxTextureSize = 0x0D33,
    MaxTextureStackDepth = 0x0D39,
    MaxViewportDims = 0x0D3A,
    ModelviewMatrix = 0x0BA6,
    ModelviewStackDepth = 0x0BA3,
    Normalize = 0x0BA1,
    NormalArray = 0x8075,
    NormalArrayStride = 0x807F,
    NormalArrayType = 0x807E,
    NumCompressedTextureFormats = 0x86A2,
    PackAlignment = 0x0D05,
    PerspectiveCorrectionHint = 0x0C50,
    PointFadeThresholdSize = 0x8128,
    PointSize = 0x0B11,
    PointSmooth = 0x0B10,
    PointSmoothHint = 0x0C51,
    PolygonOffsetFactor = 0x8038,
    PolygonOffsetFill = 0x8037,
    PolygonOffsetUnits = 0x2A00,
    ProjectionMatrix = 0x0BA7,
    ProjectionStackDepth = 0x0BA4,
    RedBits = 0x0D52,
    Samples = 0x80A9,
    SampleBuffers = 0x80A8,
    SampleCoverageInvert = 0x80AB,
    SampleCoverageValue = 0x80AA,
    ScissorBox = 0x0C10,
    ScissorTest = 0x0C11,
    ShadeModel = 0x0B54,
    SmoothLineWidthRange = 0x0B22,
    SmoothPointSizeRange = 0x0B12,
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
    TextureCoordArray = 0x8078,
    TextureCoordArraySize = 0x8088,
    TextureCoordArrayStride = 0x808A,
    TextureCoordArrayType = 0x8089,
    TextureMatrix = 0x0BA8,
    TextureStackDepth = 0x0BA5,
    UnpackAlignment = 0x0CF5,
    VertexArray = 0x8074,
    VertexArraySize = 0x807A,
    VertexArrayStride = 0x807C,
    VertexArrayType = 0x807B,
    Viewport = 0x0BA2,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum GetPName

enum class GetPointervPName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    ColorArrayPointer = 0x8090,
    NormalArrayPointer = 0x808F,
    TextureCoordArrayPointer = 0x8092,
    VertexArrayPointer = 0x808E,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum GetPointervPName

enum class GetTextureParameter : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    TextureMagFilter = 0x2800,
    TextureMinFilter = 0x2801,
    TextureWrapS = 0x2802,
    TextureWrapT = 0x2803,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum GetTextureParameter

enum class HintMode : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    DontCare = 0x1100,
    Fastest = 0x1101,
    Nicest = 0x1102,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum HintMode

enum class HintTarget : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    FogHint = 0x0C54,
    GenerateMipmapHint = 0x8192,
    LineSmoothHint = 0x0C52,
    PerspectiveCorrectionHint = 0x0C50,
    PointSmoothHint = 0x0C51,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum HintTarget

enum class HistogramTargetEXT : core::EnumType
{
}; // enum HistogramTargetEXT

enum class IndexPointerType : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Float = 0x1406,
    Short = 0x1402,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum IndexPointerType

enum class InterleavedArrayFormat : core::EnumType
{
}; // enum InterleavedArrayFormat

enum class LightEnvModeSGIX : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Add = 0x0104,
    Modulate = 0x2100,
    Replace = 0x1E01,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum LightEnvModeSGIX

enum class LightEnvParameterSGIX : core::EnumType
{
}; // enum LightEnvParameterSGIX

enum class LightModelColorControl : core::EnumType
{
}; // enum LightModelColorControl

enum class LightModelParameter : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    LightModelAmbient = 0x0B53,
    LightModelTwoSide = 0x0B52,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum LightModelParameter

enum class LightName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Light0 = 0x4000,
    Light1 = 0x4001,
    Light2 = 0x4002,
    Light3 = 0x4003,
    Light4 = 0x4004,
    Light5 = 0x4005,
    Light6 = 0x4006,
    Light7 = 0x4007,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum LightName

enum class LightParameter : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Ambient = 0x1200,
    ConstantAttenuation = 0x1207,
    Diffuse = 0x1201,
    LinearAttenuation = 0x1208,
    Position = 0x1203,
    QuadraticAttenuation = 0x1209,
    Specular = 0x1202,
    SpotCutoff = 0x1206,
    SpotDirection = 0x1204,
    SpotExponent = 0x1205,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum LightParameter

enum class ListMode : core::EnumType
{
}; // enum ListMode

enum class ListNameType : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Byte = 0x1400,
    Float = 0x1406,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedShort = 0x1403,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum ListNameType

enum class ListParameterName : core::EnumType
{
}; // enum ListParameterName

enum class LogicOp : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    And = 0x1501,
    AndInverted = 0x1504,
    AndReverse = 0x1502,
    Clear = 0x1500,
    Copy = 0x1503,
    CopyInverted = 0x150C,
    Equiv = 0x1509,
    Invert = 0x150A,
    Nand = 0x150E,
    Noop = 0x1505,
    Nor = 0x1508,
    Or = 0x1507,
    OrInverted = 0x150D,
    OrReverse = 0x150B,
    Set = 0x150F,
    Xor = 0x1506,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
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
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Back = 0x0405,
    Front = 0x0404,
    FrontAndBack = 0x0408,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum MaterialFace

enum class MaterialParameter : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Ambient = 0x1200,
    AmbientAndDiffuse = 0x1602,
    Diffuse = 0x1201,
    Emission = 0x1600,
    Shininess = 0x1601,
    Specular = 0x1202,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum MaterialParameter

enum class MatrixMode : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Modelview = 0x1700,
    Projection = 0x1701,
    Texture = 0x1702,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
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
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Byte = 0x1400,
    Float = 0x1406,
    Short = 0x1402,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum NormalPointerType

enum class PixelCopyType : core::EnumType
{
}; // enum PixelCopyType

enum class PixelFormat : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Alpha = 0x1906,
    Luminance = 0x1909,
    LuminanceAlpha = 0x190A,
    Rgb = 0x1907,
    Rgba = 0x1908,
    UnsignedShort = 0x1403,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum PixelFormat

enum class InternalFormat : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Rgb = 0x1907,
    Rgba = 0x1908,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum InternalFormat

enum class PixelMap : core::EnumType
{
}; // enum PixelMap

enum class PixelStoreParameter : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    PackAlignment = 0x0D05,
    UnpackAlignment = 0x0CF5,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum PixelStoreParameter

enum class PixelStoreResampleMode : core::EnumType
{
}; // enum PixelStoreResampleMode

enum class PixelStoreSubsampleRate : core::EnumType
{
}; // enum PixelStoreSubsampleRate

enum class PixelTexGenMode : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Luminance = 0x1909,
    LuminanceAlpha = 0x190A,
    Rgb = 0x1907,
    Rgba = 0x1908,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum PixelTexGenMode

enum class PixelTexGenParameterNameSGIS : core::EnumType
{
}; // enum PixelTexGenParameterNameSGIS

enum class PixelTransferParameter : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    AlphaScale = 0x0D1C,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum PixelTransferParameter

enum class PixelType : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Byte = 0x1400,
    Float = 0x1406,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedShort = 0x1403,
    UnsignedShort4444 = 0x8033,
    UnsignedShort5551 = 0x8034,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum PixelType

enum class PointParameterNameSGIS : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    PointDistanceAttenuation = 0x8129,
    PointFadeThresholdSize = 0x8128,
    PointSizeMax = 0x8127,
    PointSizeMin = 0x8126,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum PointParameterNameSGIS

enum class PolygonMode : core::EnumType
{
}; // enum PolygonMode

enum class PrimitiveType : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Lines = 0x0001,
    LineLoop = 0x0002,
    LineStrip = 0x0003,
    Points = 0x0000,
    Triangles = 0x0004,
    TriangleFan = 0x0006,
    TriangleStrip = 0x0005,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum PrimitiveType

enum class OcclusionQueryEventMaskAMD : core::EnumType
{
}; // enum OcclusionQueryEventMaskAMD
constexpr core::Bitfield<OcclusionQueryEventMaskAMD> operator|(OcclusionQueryEventMaskAMD left, OcclusionQueryEventMaskAMD right) { return core::Bitfield<OcclusionQueryEventMaskAMD>(left) | right;}

enum class ReadBufferMode : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Back = 0x0405,
    Front = 0x0404,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
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
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Flat = 0x1D00,
    Smooth = 0x1D01,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum ShadingModel

enum class StencilFaceDirection : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Back = 0x0405,
    Front = 0x0404,
    FrontAndBack = 0x0408,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum StencilFaceDirection

enum class StencilFunction : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Always = 0x0207,
    Equal = 0x0202,
    Gequal = 0x0206,
    Greater = 0x0204,
    Lequal = 0x0203,
    Less = 0x0201,
    Never = 0x0200,
    Notequal = 0x0205,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum StencilFunction

enum class StencilOp : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Decr = 0x1E03,
    Incr = 0x1E02,
    Invert = 0x150A,
    Keep = 0x1E00,
    Replace = 0x1E01,
    Zero = 0,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum StencilOp

enum class StringName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Extensions = 0x1F03,
    Renderer = 0x1F01,
    Vendor = 0x1F00,
    Version = 0x1F02,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum StringName

enum class SyncObjectMask : core::EnumType
{
}; // enum SyncObjectMask
constexpr core::Bitfield<SyncObjectMask> operator|(SyncObjectMask left, SyncObjectMask right) { return core::Bitfield<SyncObjectMask>(left) | right;}

enum class TexCoordPointerType : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Float = 0x1406,
    Short = 0x1402,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum TexCoordPointerType

enum class TextureCoordName : core::EnumType
{
}; // enum TextureCoordName

enum class TextureEnvMode : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Add = 0x0104,
    Blend = 0x0BE2,
    Decal = 0x2101,
    Modulate = 0x2100,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum TextureEnvMode

enum class TextureEnvParameter : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    TextureEnvColor = 0x2201,
    TextureEnvMode = 0x2200,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum TextureEnvParameter

enum class TextureEnvTarget : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    TextureEnv = 0x2300,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
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
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Linear = 0x2601,
    Nearest = 0x2600,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum TextureMagFilter

enum class TextureMinFilter : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Linear = 0x2601,
    LinearMipmapLinear = 0x2703,
    LinearMipmapNearest = 0x2701,
    Nearest = 0x2600,
    NearestMipmapLinear = 0x2702,
    NearestMipmapNearest = 0x2700,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum TextureMinFilter

enum class TextureParameterName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    GenerateMipmap = 0x8191,
    TextureMagFilter = 0x2800,
    TextureMinFilter = 0x2801,
    TextureWrapS = 0x2802,
    TextureWrapT = 0x2803,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum TextureParameterName

enum class TextureStorageMaskAMD : core::EnumType
{
}; // enum TextureStorageMaskAMD
constexpr core::Bitfield<TextureStorageMaskAMD> operator|(TextureStorageMaskAMD left, TextureStorageMaskAMD right) { return core::Bitfield<TextureStorageMaskAMD>(left) | right;}

enum class TextureTarget : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Texture2d = 0x0DE1,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum TextureTarget

enum class TextureWrapMode : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    ClampToEdge = 0x812F,
    Repeat = 0x2901,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum TextureWrapMode

enum class UseProgramStageMask : core::EnumType
{
}; // enum UseProgramStageMask
constexpr core::Bitfield<UseProgramStageMask> operator|(UseProgramStageMask left, UseProgramStageMask right) { return core::Bitfield<UseProgramStageMask>(left) | right;}

enum class VertexPointerType : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Float = 0x1406,
    Short = 0x1402,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum VertexPointerType

enum class FramebufferAttachment : core::EnumType
{
}; // enum FramebufferAttachment

enum class RenderbufferTarget : core::EnumType
{
}; // enum RenderbufferTarget

enum class FramebufferTarget : core::EnumType
{
}; // enum FramebufferTarget

enum class TextureUnit : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
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
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum TextureUnit

enum class ConditionalRenderMode : core::EnumType
{
}; // enum ConditionalRenderMode

enum class FragmentOpATI : core::EnumType
{
}; // enum FragmentOpATI

enum class FramebufferStatus : core::EnumType
{
}; // enum FramebufferStatus

enum class GraphicsResetStatus : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    NoError = 0,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
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
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Invert = 0x150A,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum PathFillMode

enum class ColorTableTarget : core::EnumType
{
}; // enum ColorTableTarget

enum class VertexBufferObjectParameter : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    BufferSize = 0x8764,
    BufferUsage = 0x8765,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum VertexBufferObjectParameter

enum class RenderbufferParameterName : core::EnumType
{
}; // enum RenderbufferParameterName

enum class VertexBufferObjectUsage : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    DynamicDraw = 0x88E8,
    StaticDraw = 0x88E4,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum VertexBufferObjectUsage

enum class FramebufferParameterName : core::EnumType
{
}; // enum FramebufferParameterName

enum class ProgramParameterPName : core::EnumType
{
}; // enum ProgramParameterPName

enum class BlendingFactor : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    DstAlpha = 0x0304,
    DstColor = 0x0306,
    One = 1,
    OneMinusDstAlpha = 0x0305,
    OneMinusDstColor = 0x0307,
    OneMinusSrcAlpha = 0x0303,
    OneMinusSrcColor = 0x0301,
    Src1Alpha = 0x8589,
    SrcAlpha = 0x0302,
    SrcAlphaSaturate = 0x0308,
    SrcColor = 0x0300,
    Zero = 0,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum BlendingFactor

enum class BindTransformFeedbackTarget : core::EnumType
{
}; // enum BindTransformFeedbackTarget

enum class BlitFramebufferFilter : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Linear = 0x2601,
    Nearest = 0x2600,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum BlitFramebufferFilter

enum class BufferStorageTarget : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    ArrayBuffer = 0x8892,
    ElementArrayBuffer = 0x8893,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum BufferStorageTarget

enum class CheckFramebufferStatusTarget : core::EnumType
{
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
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    ArrayBuffer = 0x8892,
    ElementArrayBuffer = 0x8893,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum CopyBufferSubDataTarget

enum class ShaderType : core::EnumType
{
}; // enum ShaderType

enum class DebugSource : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    DontCare = 0x1100,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum DebugSource

enum class DebugType : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    DontCare = 0x1100,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum DebugType

enum class DebugSeverity : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    DontCare = 0x1100,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum DebugSeverity

enum class SyncCondition : core::EnumType
{
}; // enum SyncCondition

enum class FogPName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    FogDensity = 0x0B62,
    FogEnd = 0x0B64,
    FogMode = 0x0B65,
    FogStart = 0x0B63,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
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
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    TextureMagFilter = 0x2800,
    TextureMinFilter = 0x2801,
    TextureWrapS = 0x2802,
    TextureWrapT = 0x2803,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum SamplerParameterName

enum class VertexProvokingMode : core::EnumType
{
}; // enum VertexProvokingMode

enum class PatchParameterName : core::EnumType
{
}; // enum PatchParameterName

enum class ObjectIdentifier : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Texture = 0x1702,
    VertexArray = 0x8074,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum ObjectIdentifier

enum class ColorBuffer : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Back = 0x0405,
    Front = 0x0404,
    FrontAndBack = 0x0408,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum ColorBuffer

enum class MapQuery : core::EnumType
{
}; // enum MapQuery

enum class VertexArrayPName : core::EnumType
{
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
}; // enum VertexAttribEnum

enum class VertexAttribType : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Byte = 0x1400,
    Fixed = 0x140C,
    Float = 0x1406,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedShort = 0x1403,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum VertexAttribType

enum class AttributeType : core::EnumType
{
}; // enum AttributeType

enum class InternalFormatPName : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    GenerateMipmap = 0x8191,
    Samples = 0x80A9,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum InternalFormatPName

enum class FramebufferAttachmentParameterName : core::EnumType
{
}; // enum FramebufferAttachmentParameterName

enum class ProgramInterfacePName : core::EnumType
{
}; // enum ProgramInterfacePName

enum class PrecisionType : core::EnumType
{
}; // enum PrecisionType

enum class VertexAttribPointerType : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Byte = 0x1400,
    Fixed = 0x140C,
    Float = 0x1406,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedShort = 0x1403,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum VertexAttribPointerType

enum class SubroutineParameterName : core::EnumType
{
}; // enum SubroutineParameterName

enum class GetFramebufferParameter : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Samples = 0x80A9,
    SampleBuffers = 0x80A8,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
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
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    PrimaryColor = 0x8577,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum PathColor

enum class PathGenMode : core::EnumType
{
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
    Constant = 0x8576,
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
}; // enum PathGenMode

enum class TextureLayout : core::EnumType
{
}; // enum TextureLayout

enum class PathTransformType : core::EnumType
{
}; // enum PathTransformType

enum class PathElementType : core::EnumType
{
}; // enum PathElementType

enum class PathCoverMode : core::EnumType
{
}; // enum PathCoverMode

enum class PathFontStyle : core::EnumType
{
}; // enum PathFontStyle

enum class PathMetricMask : core::EnumType
{
}; // enum PathMetricMask

enum class PathListMode : core::EnumType
{
}; // enum PathListMode

enum class ProgramPropertyARB : core::EnumType
{
}; // enum ProgramPropertyARB

// commands
#if defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)
extern void bindBuffer(BufferTargetARB target, uint32_t buffer);
extern void lightx(LightName light, LightParameter pname, core::Fixed param);
extern void getLight(LightName light, LightParameter pname, float *params);
extern void sampleCoverage(float value, bool invert);
extern void enable(EnableCap cap);
extern void clearDepthx(core::Fixed depth);
extern void texCoordPointer(int32_t size, TexCoordPointerType type, core::SizeType stride, const void *pointer);
extern void clearStencil(int32_t s);
extern void getTexParameter(TextureTarget target, GetTextureParameter pname, int32_t *params);
extern void lineWidthx(core::Fixed width);
extern void copyTexImage2D(TextureTarget target, int32_t level, InternalFormat internalformat, int32_t x, int32_t y, core::SizeType width, core::SizeType height, int32_t border);
extern void texParameterxv(TextureTarget target, GetTextureParameter pname, const core::Fixed *params);
extern void getFixe(GetPName pname, core::Fixed *params);
extern void normalPointer(NormalPointerType type, core::SizeType stride, const void *pointer);
extern void texEnv(TextureEnvTarget target, TextureEnvParameter pname, const float *params);
extern void deleteBuffers(core::SizeType n, const uint32_t *buffers);
extern void getTexEnvxv(TextureEnvTarget target, TextureEnvParameter pname, core::Fixed *params);
extern void depthMask(bool flag);
extern void getTexParameter(TextureTarget target, GetTextureParameter pname, float *params);
extern void getLightxv(LightName light, LightParameter pname, core::Fixed *params);
extern void lightModel(LightModelParameter pname, float param);
extern void pointParameterxv(PointParameterNameARB pname, const core::Fixed *params);
extern void rotate(float angle, float x, float y, float z);
extern void texEnvx(TextureEnvTarget target, TextureEnvParameter pname, core::Fixed param);
extern void loadMatrix(const float *m);
extern void hint(HintTarget target, HintMode mode);
extern bool isTexture(uint32_t texture);
extern void lightModelxv(LightModelParameter pname, const core::Fixed *param);
extern void colorPointer(int32_t size, ColorPointerType type, core::SizeType stride, const void *pointer);
extern void readPixel(int32_t x, int32_t y, core::SizeType width, core::SizeType height, PixelFormat format, PixelType type, void *pixels);
extern void texEnv(TextureEnvTarget target, TextureEnvParameter pname, const int32_t *params);
extern void clearDepth(float d);
extern void lightxv(LightName light, LightParameter pname, const core::Fixed *params);
extern void deleteTextures(core::SizeType n, const uint32_t *textures);
extern void translatex(core::Fixed x, core::Fixed y, core::Fixed z);
extern void texSubImage2D(TextureTarget target, int32_t level, int32_t xoffset, int32_t yoffset, core::SizeType width, core::SizeType height, PixelFormat format, PixelType type, const void *pixels);
extern void getClipPlane(ClipPlaneName plane, float *equation);
extern void logicOp(LogicOp opcode);
extern void material(MaterialFace face, MaterialParameter pname, const float *params);
extern void loadMatrixx(const core::Fixed *m);
extern void scissor(int32_t x, int32_t y, core::SizeType width, core::SizeType height);
extern void blendFunc(BlendingFactor sfactor, BlendingFactor dfactor);
extern void lightModelx(LightModelParameter pname, core::Fixed param);
extern void flush();
extern void fogx(FogPName pname, core::Fixed param);
extern void getBooleanv(GetPName pname, bool *data);
extern void stencilFunc(StencilFunction func, int32_t ref, uint32_t mask);
extern void finish();
extern void fog(FogParameter pname, const float *params);
extern void pointParameter(PointParameterNameARB pname, float param);
extern void getMaterialxv(MaterialFace face, MaterialParameter pname, core::Fixed *params);
extern void texParameter(TextureTarget target, TextureParameterName pname, int32_t param);
extern void getTexParameterxv(TextureTarget target, GetTextureParameter pname, core::Fixed *params);
extern void compressedTexImage2D(TextureTarget target, int32_t level, InternalFormat internalformat, core::SizeType width, core::SizeType height, int32_t border, core::SizeType imageSize, const void *data);
extern void bufferData(BufferTargetARB target, std::size_t size, const void *data, BufferUsageARB usage);
extern void multMatrix(const float *m);
extern void light(LightName light, LightParameter pname, float param);
extern void bufferSubData(BufferTargetARB target, std::intptr_t offset, std::size_t size, const void *data);
extern void drawElements(PrimitiveType mode, core::SizeType count, DrawElementsType type, const void *indices);
extern void genTextures(core::SizeType n, uint32_t *textures);
extern core::EnumType getError();
extern void alphaFunc(AlphaFunction func, float ref);
extern void translate(float x, float y, float z);
extern void texEnvxv(TextureEnvTarget target, TextureEnvParameter pname, const core::Fixed *params);
extern void frontFace(FrontFaceDirection mode);
extern void texParameter(TextureTarget target, TextureParameterName pname, const int32_t *params);
extern void frustumx(core::Fixed l, core::Fixed r, core::Fixed b, core::Fixed t, core::Fixed n, core::Fixed f);
extern void getIntegerv(GetPName pname, int32_t *data);
extern void normal3(float nx, float ny, float nz);
extern void colorMask(bool red, bool green, bool blue, bool alpha);
extern void clipPlanex(ClipPlaneName plane, const core::Fixed *equation);
extern void materialxv(MaterialFace face, MaterialParameter pname, const core::Fixed *param);
extern void scale(float x, float y, float z);
extern void lightModel(LightModelParameter pname, const float *params);
extern void color4(float red, float green, float blue, float alpha);
extern void multiTexCoord4(TextureUnit target, float s, float t, float r, float q);
extern void fog(FogParameter pname, float param);
extern void matrixMode(MatrixMode mode);
extern void polygonOffset(float factor, float units);
extern void texImage2D(TextureTarget target, int32_t level, InternalFormat internalformat, core::SizeType width, core::SizeType height, int32_t border, PixelFormat format, PixelType type, const void *pixels);
extern void texParameterx(TextureTarget target, GetTextureParameter pname, core::Fixed param);
extern void getTexEnv(TextureEnvTarget target, TextureEnvParameter pname, int32_t *params);
extern void shadeModel(ShadingModel mode);
extern void clearColorx(core::Fixed red, core::Fixed green, core::Fixed blue, core::Fixed alpha);
extern void loadIdentity();
extern void genBuffers(core::SizeType n, uint32_t *buffers);
extern void cullFace(CullFaceMode mode);
extern void disable(EnableCap cap);
extern void clear(core::Bitfield<ClearBufferMask> mask);
extern void polygonOffsetx(core::Fixed factor, core::Fixed units);
extern void bindTexture(TextureTarget target, uint32_t texture);
extern void clipPlane(ClipPlaneName p, const float *eqn);
extern void color4x(core::Fixed red, core::Fixed green, core::Fixed blue, core::Fixed alpha);
extern void getMaterial(MaterialFace face, MaterialParameter pname, float *params);
extern void depthRangex(core::Fixed n, core::Fixed f);
extern void activeTexture(TextureUnit texture);
extern void rotatex(core::Fixed angle, core::Fixed x, core::Fixed y, core::Fixed z);
extern void scalex(core::Fixed x, core::Fixed y, core::Fixed z);
extern void texEnv(TextureEnvTarget target, TextureEnvParameter pname, float param);
extern const uint8_t *getString(StringName name);
extern void lineWidth(float width);
extern void multMatrixx(const core::Fixed *m);
extern void fogxv(FogPName pname, const core::Fixed *param);
extern void material(MaterialFace face, MaterialParameter pname, float param);
extern void stencilMask(uint32_t mask);
extern void pointParameterx(PointParameterNameARB pname, core::Fixed param);
extern void getPointerv(GetPointervPName pname, void **params);
extern void materialx(MaterialFace face, MaterialParameter pname, core::Fixed param);
extern void clearColor(float red, float green, float blue, float alpha);
extern void stencilOp(StencilOp fail, StencilOp zfail, StencilOp zpass);
extern void texParameter(TextureTarget target, TextureParameterName pname, float param);
extern void normal3x(core::Fixed nx, core::Fixed ny, core::Fixed nz);
extern void getClipPlanex(ClipPlaneName plane, core::Fixed *equation);
extern void drawArrays(PrimitiveType mode, int32_t first, core::SizeType count);
extern void vertexPointer(int32_t size, VertexPointerType type, core::SizeType stride, const void *pointer);
extern bool isEnable(EnableCap cap);
extern void alphaFuncx(AlphaFunction func, core::Fixed ref);
extern void enableClientState(EnableCap array);
extern void pointSizex(core::Fixed size);
extern void clientActiveTexture(TextureUnit texture);
extern void depthFunc(DepthFunction func);
extern void ortho(float l, float r, float b, float t, float n, float f);
extern void color4(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha);
extern bool isBuffer(uint32_t buffer);
extern void compressedTexSubImage2D(TextureTarget target, int32_t level, int32_t xoffset, int32_t yoffset, core::SizeType width, core::SizeType height, PixelFormat format, core::SizeType imageSize, const void *data);
extern void pointSize(float size);
extern void copyTexSubImage2D(TextureTarget target, int32_t level, int32_t xoffset, int32_t yoffset, int32_t x, int32_t y, core::SizeType width, core::SizeType height);
extern void viewport(int32_t x, int32_t y, core::SizeType width, core::SizeType height);
extern void frustum(float l, float r, float b, float t, float n, float f);
extern void multiTexCoord4x(TextureUnit texture, core::Fixed s, core::Fixed t, core::Fixed r, core::Fixed q);
extern void light(LightName light, LightParameter pname, const float *params);
extern void texParameter(TextureTarget target, TextureParameterName pname, const float *params);
extern void getTexEnv(TextureEnvTarget target, TextureEnvParameter pname, float *params);
extern void pointParameter(PointParameterNameARB pname, const float *params);
extern void popMatrix();
extern void pushMatrix();
extern void disableClientState(EnableCap array);
extern void texEnv(TextureEnvTarget target, TextureEnvParameter pname, int32_t param);
extern void orthox(core::Fixed l, core::Fixed r, core::Fixed b, core::Fixed t, core::Fixed n, core::Fixed f);
extern void pixelStore(PixelStoreParameter pname, int32_t param);
extern void getFloatv(GetPName pname, float *data);
extern void sampleCoveragex(GLclampx value, bool invert);
extern void depthRange(float n, float f);
extern void getBufferParameter(BufferTargetARB target, BufferPNameARB pname, int32_t *params);
#endif // defined(API_LEVEL_GL_VERSION_ES_CM_1_0_common)

}
