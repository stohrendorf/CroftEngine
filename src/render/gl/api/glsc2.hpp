#pragma once
#include "soglb_core.hpp"
#include "glsc2_enabled_features.hpp"
namespace glsc2
{
// API feature levels
// #define API_LEVEL_GL_SC_VERSION_2_0

// enums
enum class AlphaFunction : core::EnumType
{
    Always = 0x0207,
    Equal = 0x0202,
    Gequal = 0x0206,
    Greater = 0x0204,
    Lequal = 0x0203,
    Less = 0x0201,
    Never = 0x0200,
    Notequal = 0x0205,
};

enum class AttribMask : core::EnumType
{
    ColorBufferBit = 0x00004000,
    DepthBufferBit = 0x00000100,
    StencilBufferBit = 0x00000400,
};
constexpr core::Bitfield<AttribMask> operator|(AttribMask left, AttribMask right) { return core::Bitfield<AttribMask>(left) | right;}

enum class BlendEquationModeEXT : core::EnumType
{
    FuncAdd = 0x8006,
    FuncReverseSubtract = 0x800B,
    FuncSubtract = 0x800A,
};

enum class BlendingFactor : core::EnumType
{
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
};

enum class BlitFramebufferFilter : core::EnumType
{
    Linear = 0x2601,
    Nearest = 0x2600,
};

enum class Boolean : core::EnumType
{
    False = 0,
    True = 1,
};

enum class BufferPNameARB : core::EnumType
{
    BufferSize = 0x8764,
    BufferUsage = 0x8765,
};

enum class BufferStorageTarget : core::EnumType
{
    ArrayBuffer = 0x8892,
    ElementArrayBuffer = 0x8893,
};

enum class BufferTargetARB : core::EnumType
{
    ArrayBuffer = 0x8892,
    ElementArrayBuffer = 0x8893,
};

enum class BufferUsageARB : core::EnumType
{
    DynamicDraw = 0x88E8,
    StaticDraw = 0x88E4,
    StreamDraw = 0x88E0,
};

enum class CheckFramebufferStatusTarget : core::EnumType
{
    Framebuffer = 0x8D40,
};

enum class ClampColorModeARB : core::EnumType
{
    False = 0,
    True = 1,
};

enum class ClearBufferMask : core::EnumType
{
    ColorBufferBit = 0x00004000,
    DepthBufferBit = 0x00000100,
    StencilBufferBit = 0x00000400,
};
constexpr core::Bitfield<ClearBufferMask> operator|(ClearBufferMask left, ClearBufferMask right) { return core::Bitfield<ClearBufferMask>(left) | right;}

enum class ColorBuffer : core::EnumType
{
    Back = 0x0405,
    ColorAttachment0 = 0x8CE0,
    Front = 0x0404,
    FrontAndBack = 0x0408,
    None = 0,
};

enum class ColorMaterialFace : core::EnumType
{
    Back = 0x0405,
    Front = 0x0404,
    FrontAndBack = 0x0408,
};

enum class ColorPointerType : core::EnumType
{
    Byte = 0x1400,
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
};

enum class CombinerBiasNV : core::EnumType
{
    None = 0,
};

enum class CombinerScaleNV : core::EnumType
{
    None = 0,
};

enum class CopyBufferSubDataTarget : core::EnumType
{
    ArrayBuffer = 0x8892,
    ElementArrayBuffer = 0x8893,
};

enum class CullFaceMode : core::EnumType
{
    Back = 0x0405,
    Front = 0x0404,
    FrontAndBack = 0x0408,
};

enum class DebugSeverity : core::EnumType
{
    DontCare = 0x1100,
};

enum class DebugSource : core::EnumType
{
    DontCare = 0x1100,
};

enum class DebugType : core::EnumType
{
    DontCare = 0x1100,
};

enum class DepthFunction : core::EnumType
{
    Always = 0x0207,
    Equal = 0x0202,
    Gequal = 0x0206,
    Greater = 0x0204,
    Lequal = 0x0203,
    Less = 0x0201,
    Never = 0x0200,
    Notequal = 0x0205,
};

enum class DrawBufferMode : core::EnumType
{
    Back = 0x0405,
    ColorAttachment0 = 0x8CE0,
    Front = 0x0404,
    FrontAndBack = 0x0408,
    None = 0,
};

enum class DrawElementsType : core::EnumType
{
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
};

enum class EnableCap : core::EnumType
{
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
};

enum class ErrorCode : core::EnumType
{
    InvalidEnum = 0x0500,
    InvalidFramebufferOperation = 0x0506,
    InvalidOperation = 0x0502,
    InvalidValue = 0x0501,
    NoError = 0,
    OutOfMemory = 0x0505,
};

enum class FogCoordinatePointerType : core::EnumType
{
    Float = 0x1406,
};

enum class FogMode : core::EnumType
{
    Linear = 0x2601,
};

enum class FogPointerTypeEXT : core::EnumType
{
    Float = 0x1406,
};

enum class FogPointerTypeIBM : core::EnumType
{
    Float = 0x1406,
};

enum class FramebufferAttachment : core::EnumType
{
    ColorAttachment0 = 0x8CE0,
    DepthAttachment = 0x8D00,
};

enum class FramebufferAttachmentParameterName : core::EnumType
{
    FramebufferAttachmentObjectName = 0x8CD1,
    FramebufferAttachmentTextureLevel = 0x8CD2,
};

enum class FramebufferStatus : core::EnumType
{
    FramebufferComplete = 0x8CD5,
    FramebufferIncompleteAttachment = 0x8CD6,
    FramebufferIncompleteMissingAttachment = 0x8CD7,
    FramebufferUndefined = 0x8219,
    FramebufferUnsupported = 0x8CDD,
};

enum class FramebufferTarget : core::EnumType
{
    Framebuffer = 0x8D40,
};

enum class FrontFaceDirection : core::EnumType
{
    Ccw = 0x0901,
    Cw = 0x0900,
};

enum class GetFramebufferParameter : core::EnumType
{
    ImplementationColorReadFormat = 0x8B9B,
    ImplementationColorReadType = 0x8B9A,
    Samples = 0x80A9,
    SampleBuffers = 0x80A8,
};

enum class GetPName : core::EnumType
{
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
};

enum class GetTextureParameter : core::EnumType
{
    TextureMagFilter = 0x2800,
    TextureMinFilter = 0x2801,
    TextureWrapS = 0x2802,
    TextureWrapT = 0x2803,
};

enum class GlslTypeToken : core::EnumType
{
    Float = 0x1406,
    Int = 0x1404,
    Sampler2d = 0x8B5E,
    UnsignedInt = 0x1405,
};

enum class GraphicsResetStatus : core::EnumType
{
    GuiltyContextReset = 0x8253,
    InnocentContextReset = 0x8254,
    NoError = 0,
    UnknownContextReset = 0x8255,
};

enum class HintMode : core::EnumType
{
    DontCare = 0x1100,
    Fastest = 0x1101,
    Nicest = 0x1102,
};

enum class HintTarget : core::EnumType
{
    GenerateMipmapHint = 0x8192,
};

enum class IndexPointerType : core::EnumType
{
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
};

enum class InternalFormat : core::EnumType
{
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
};

enum class InternalFormatPName : core::EnumType
{
    Samples = 0x80A9,
};

enum class LightEnvModeSGIX : core::EnumType
{
    Replace = 0x1E01,
};

enum class ListNameType : core::EnumType
{
    Byte = 0x1400,
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
};

enum class LogicOp : core::EnumType
{
    Invert = 0x150A,
};

enum class MaterialFace : core::EnumType
{
    Back = 0x0405,
    Front = 0x0404,
    FrontAndBack = 0x0408,
};

enum class MatrixMode : core::EnumType
{
    Texture = 0x1702,
};

enum class NormalPointerType : core::EnumType
{
    Byte = 0x1400,
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
};

enum class ObjectIdentifier : core::EnumType
{
    Framebuffer = 0x8D40,
    Renderbuffer = 0x8D41,
    Texture = 0x1702,
};

enum class PathFillMode : core::EnumType
{
    Invert = 0x150A,
};

enum class PathFontStyle : core::EnumType
{
    None = 0,
};

enum class PathGenMode : core::EnumType
{
    None = 0,
};

enum class PathTransformType : core::EnumType
{
    None = 0,
};

enum class PixelFormat : core::EnumType
{
    Red = 0x1903,
    Rg = 0x8227,
    Rgb = 0x1907,
    Rgba = 0x1908,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
};

enum class PixelStoreParameter : core::EnumType
{
    PackAlignment = 0x0D05,
    UnpackAlignment = 0x0CF5,
};

enum class PixelTexGenMode : core::EnumType
{
    None = 0,
    Rgb = 0x1907,
    Rgba = 0x1908,
};

enum class PixelType : core::EnumType
{
    Byte = 0x1400,
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
    UnsignedShort4444 = 0x8033,
    UnsignedShort5551 = 0x8034,
};

enum class PrecisionType : core::EnumType
{
    HighFloat = 0x8DF2,
    HighInt = 0x8DF5,
    LowFloat = 0x8DF0,
    LowInt = 0x8DF3,
    MediumFloat = 0x8DF1,
    MediumInt = 0x8DF4,
};

enum class PrimitiveType : core::EnumType
{
    Lines = 0x0001,
    LineLoop = 0x0002,
    LineStrip = 0x0003,
    Points = 0x0000,
    Triangles = 0x0004,
    TriangleFan = 0x0006,
    TriangleStrip = 0x0005,
};

enum class ProgramPropertyARB : core::EnumType
{
    LinkStatus = 0x8B82,
};

enum class ReadBufferMode : core::EnumType
{
    Back = 0x0405,
    ColorAttachment0 = 0x8CE0,
    Front = 0x0404,
};

enum class RenderbufferParameterName : core::EnumType
{
    RenderbufferAlphaSize = 0x8D53,
    RenderbufferBlueSize = 0x8D52,
    RenderbufferDepthSize = 0x8D54,
    RenderbufferGreenSize = 0x8D51,
    RenderbufferHeight = 0x8D43,
    RenderbufferInternalFormat = 0x8D44,
    RenderbufferRedSize = 0x8D50,
    RenderbufferStencilSize = 0x8D55,
    RenderbufferWidth = 0x8D42,
};

enum class RenderbufferTarget : core::EnumType
{
    Renderbuffer = 0x8D41,
};

enum class SamplerParameterI : core::EnumType
{
    TextureMagFilter = 0x2800,
    TextureMinFilter = 0x2801,
    TextureWrapS = 0x2802,
    TextureWrapT = 0x2803,
};

enum class StencilFaceDirection : core::EnumType
{
    Back = 0x0405,
    Front = 0x0404,
    FrontAndBack = 0x0408,
};

enum class StencilFunction : core::EnumType
{
    Always = 0x0207,
    Equal = 0x0202,
    Gequal = 0x0206,
    Greater = 0x0204,
    Lequal = 0x0203,
    Less = 0x0201,
    Never = 0x0200,
    Notequal = 0x0205,
};

enum class StencilOp : core::EnumType
{
    Decr = 0x1E03,
    DecrWrap = 0x8508,
    Incr = 0x1E02,
    IncrWrap = 0x8507,
    Invert = 0x150A,
    Keep = 0x1E00,
    Replace = 0x1E01,
    Zero = 0,
};

enum class StringName : core::EnumType
{
    Extensions = 0x1F03,
    Renderer = 0x1F01,
    ShadingLanguageVersion = 0x8B8C,
    Vendor = 0x1F00,
    Version = 0x1F02,
};

enum class TexCoordPointerType : core::EnumType
{
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
};

enum class TextureCompareMode : core::EnumType
{
    None = 0,
};

enum class TextureEnvMode : core::EnumType
{
    Blend = 0x0BE2,
};

enum class TextureMagFilter : core::EnumType
{
    Linear = 0x2601,
    Nearest = 0x2600,
};

enum class TextureMinFilter : core::EnumType
{
    Linear = 0x2601,
    LinearMipmapLinear = 0x2703,
    LinearMipmapNearest = 0x2701,
    Nearest = 0x2600,
    NearestMipmapLinear = 0x2702,
    NearestMipmapNearest = 0x2700,
};

enum class TextureParameterName : core::EnumType
{
    TextureMagFilter = 0x2800,
    TextureMinFilter = 0x2801,
    TextureWrapS = 0x2802,
    TextureWrapT = 0x2803,
};

enum class TextureSwizzle : core::EnumType
{
    One = 1,
    Red = 0x1903,
    Zero = 0,
};

enum class TextureTarget : core::EnumType
{
    Texture2d = 0x0DE1,
};

enum class TextureUnit : core::EnumType
{
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
};

enum class TextureWrapMode : core::EnumType
{
    ClampToEdge = 0x812F,
    LinearMipmapLinear = 0x2703,
    MirroredRepeat = 0x8370,
    Repeat = 0x2901,
};

enum class UniformType : core::EnumType
{
    Float = 0x1406,
    Int = 0x1404,
    Sampler2d = 0x8B5E,
    UnsignedInt = 0x1405,
};

enum class VertexArrayPName : core::EnumType
{
    VertexAttribArrayEnabled = 0x8622,
    VertexAttribArrayNormalized = 0x886A,
    VertexAttribArraySize = 0x8623,
    VertexAttribArrayStride = 0x8624,
    VertexAttribArrayType = 0x8625,
};

enum class VertexAttribEnum : core::EnumType
{
    CurrentVertexAttrib = 0x8626,
    VertexAttribArrayBufferBinding = 0x889F,
    VertexAttribArrayEnabled = 0x8622,
    VertexAttribArrayNormalized = 0x886A,
    VertexAttribArraySize = 0x8623,
    VertexAttribArrayStride = 0x8624,
    VertexAttribArrayType = 0x8625,
};

enum class VertexAttribIType : core::EnumType
{
    Byte = 0x1400,
    Int = 0x1404,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
};

enum class VertexAttribPointerPropertyARB : core::EnumType
{
    VertexAttribArrayPointer = 0x8645,
};

enum class VertexAttribPointerType : core::EnumType
{
    Byte = 0x1400,
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
};

enum class VertexAttribPropertyARB : core::EnumType
{
    CurrentVertexAttrib = 0x8626,
    VertexAttribArrayBufferBinding = 0x889F,
    VertexAttribArrayEnabled = 0x8622,
    VertexAttribArrayNormalized = 0x886A,
    VertexAttribArraySize = 0x8623,
    VertexAttribArrayStride = 0x8624,
    VertexAttribArrayType = 0x8625,
};

enum class VertexAttribType : core::EnumType
{
    Byte = 0x1400,
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
    UnsignedByte = 0x1401,
    UnsignedInt = 0x1405,
    UnsignedShort = 0x1403,
};

enum class VertexBufferObjectParameter : core::EnumType
{
    BufferSize = 0x8764,
    BufferUsage = 0x8765,
};

enum class VertexBufferObjectUsage : core::EnumType
{
    DynamicDraw = 0x88E8,
    StaticDraw = 0x88E4,
    StreamDraw = 0x88E0,
};

enum class VertexPointerType : core::EnumType
{
    Float = 0x1406,
    Int = 0x1404,
    Short = 0x1402,
};

// commands
extern void activeTexture(TextureUnit texture);
extern void bindBuffer(BufferTargetARB target, uint32_t buffer);
extern void bindFramebuffer(FramebufferTarget target, uint32_t framebuffer);
extern void bindRenderbuffer(RenderbufferTarget target, uint32_t renderbuffer);
extern void bindTexture(TextureTarget target, uint32_t texture);
extern void blendColor(float red, float green, float blue, float alpha);
extern void blendEquation(BlendEquationModeEXT mode);
extern void blendEquationSeparate(BlendEquationModeEXT modeRGB, BlendEquationModeEXT modeAlpha);
extern void blendFunc(BlendingFactor sfactor, BlendingFactor dfactor);
extern void blendFuncSeparate(BlendingFactor sfactorRGB, BlendingFactor dfactorRGB, BlendingFactor sfactorAlpha, BlendingFactor dfactorAlpha);
extern void bufferData(BufferTargetARB target, std::size_t size, const void *data, BufferUsageARB usage);
extern void bufferSubData(BufferTargetARB target, std::intptr_t offset, std::size_t size, const void *data);
extern FramebufferStatus checkFramebufferStatus(FramebufferTarget target);
extern void clear(core::Bitfield<ClearBufferMask> mask);
extern void clearColor(float red, float green, float blue, float alpha);
extern void clearDepth(float d);
extern void clearStencil(int32_t s);
extern void colorMask(bool red, bool green, bool blue, bool alpha);
extern void compressedTexSubImage2D(TextureTarget target, int32_t level, int32_t xoffset, int32_t yoffset, core::SizeType width, core::SizeType height, PixelFormat format, core::SizeType imageSize, const void *data);
extern uint32_t createProgram();
extern void cullFace(CullFaceMode mode);
extern void depthFunc(DepthFunction func);
extern void depthMask(bool flag);
extern void depthRange(float n, float f);
extern void disable(EnableCap cap);
extern void disableVertexAttribArray(uint32_t index);
extern void drawArrays(PrimitiveType mode, int32_t first, core::SizeType count);
extern void drawRangeElements(PrimitiveType mode, uint32_t start, uint32_t end, core::SizeType count, DrawElementsType type, const void *indices);
extern void enable(EnableCap cap);
extern void enableVertexAttribArray(uint32_t index);
extern void finish();
extern void flush();
extern void framebufferRenderbuffer(FramebufferTarget target, FramebufferAttachment attachment, RenderbufferTarget renderbuffertarget, uint32_t renderbuffer);
extern void framebufferTexture2D(FramebufferTarget target, FramebufferAttachment attachment, TextureTarget textarget, uint32_t texture, int32_t level);
extern void frontFace(FrontFaceDirection mode);
extern void genBuffers(core::SizeType n, uint32_t *buffers);
extern void genFramebuffers(core::SizeType n, uint32_t *framebuffers);
extern void genRenderbuffers(core::SizeType n, uint32_t *renderbuffers);
extern void genTextures(core::SizeType n, uint32_t *textures);
extern void generateMipmap(TextureTarget target);
extern int32_t getAttribLocation(uint32_t program, const char *name);
extern void getBooleanv(GetPName pname, bool *data);
extern void getBufferParameter(BufferTargetARB target, BufferPNameARB pname, int32_t *params);
extern ErrorCode getError();
extern void getFloatv(GetPName pname, float *data);
extern void getFramebufferAttachmentParameter(FramebufferTarget target, FramebufferAttachment attachment, FramebufferAttachmentParameterName pname, int32_t *params);
extern GraphicsResetStatus getGraphicsResetStatus();
extern void getIntegerv(GetPName pname, int32_t *data);
extern void getProgram(uint32_t program, ProgramPropertyARB pname, int32_t *params);
extern void getRenderbufferParameter(RenderbufferTarget target, RenderbufferParameterName pname, int32_t *params);
extern const uint8_t *getString(StringName name);
extern void getTexParameter(TextureTarget target, GetTextureParameter pname, float *params);
extern void getTexParameter(TextureTarget target, GetTextureParameter pname, int32_t *params);
extern int32_t getUniformLocation(uint32_t program, const char *name);
extern void getVertexAttribPointerv(uint32_t index, VertexAttribPointerPropertyARB pname, void **pointer);
extern void getVertexAttrib(uint32_t index, VertexAttribPropertyARB pname, float *params);
extern void getVertexAttrib(uint32_t index, VertexAttribPropertyARB pname, int32_t *params);
extern void getnUniform(uint32_t program, int32_t location, core::SizeType bufSize, float *params);
extern void getnUniform(uint32_t program, int32_t location, core::SizeType bufSize, int32_t *params);
extern void hint(HintTarget target, HintMode mode);
extern bool isEnable(EnableCap cap);
extern void lineWidth(float width);
extern void pixelStore(PixelStoreParameter pname, int32_t param);
extern void polygonOffset(float factor, float units);
extern void programBinary(uint32_t program, core::EnumType binaryFormat, const void *binary, core::SizeType length);
extern void readnPixel(int32_t x, int32_t y, core::SizeType width, core::SizeType height, PixelFormat format, PixelType type, core::SizeType bufSize, void *data);
extern void renderbufferStorage(RenderbufferTarget target, InternalFormat internalformat, core::SizeType width, core::SizeType height);
extern void sampleCoverage(float value, bool invert);
extern void scissor(int32_t x, int32_t y, core::SizeType width, core::SizeType height);
extern void stencilFunc(StencilFunction func, int32_t ref, uint32_t mask);
extern void stencilFuncSeparate(StencilFaceDirection face, StencilFunction func, int32_t ref, uint32_t mask);
extern void stencilMask(uint32_t mask);
extern void stencilMaskSeparate(StencilFaceDirection face, uint32_t mask);
extern void stencilOp(StencilOp fail, StencilOp zfail, StencilOp zpass);
extern void stencilOpSeparate(StencilFaceDirection face, StencilOp sfail, StencilOp dpfail, StencilOp dppass);
extern void texParameter(TextureTarget target, TextureParameterName pname, float param);
extern void texParameter(TextureTarget target, TextureParameterName pname, const float *params);
extern void texParameter(TextureTarget target, TextureParameterName pname, int32_t param);
extern void texParameter(TextureTarget target, TextureParameterName pname, const int32_t *params);
extern void texStorage2D(TextureTarget target, core::SizeType levels, InternalFormat internalformat, core::SizeType width, core::SizeType height);
extern void texSubImage2D(TextureTarget target, int32_t level, int32_t xoffset, int32_t yoffset, core::SizeType width, core::SizeType height, PixelFormat format, PixelType type, const void *pixels);
extern void uniform1(int32_t location, float v0);
extern void uniform1(int32_t location, core::SizeType count, const float *value);
extern void uniform1(int32_t location, int32_t v0);
extern void uniform1(int32_t location, core::SizeType count, const int32_t *value);
extern void uniform2(int32_t location, float v0, float v1);
extern void uniform2(int32_t location, core::SizeType count, const float *value);
extern void uniform2(int32_t location, int32_t v0, int32_t v1);
extern void uniform2(int32_t location, core::SizeType count, const int32_t *value);
extern void uniform3(int32_t location, float v0, float v1, float v2);
extern void uniform3(int32_t location, core::SizeType count, const float *value);
extern void uniform3(int32_t location, int32_t v0, int32_t v1, int32_t v2);
extern void uniform3(int32_t location, core::SizeType count, const int32_t *value);
extern void uniform4(int32_t location, float v0, float v1, float v2, float v3);
extern void uniform4(int32_t location, core::SizeType count, const float *value);
extern void uniform4(int32_t location, int32_t v0, int32_t v1, int32_t v2, int32_t v3);
extern void uniform4(int32_t location, core::SizeType count, const int32_t *value);
extern void uniformMatrix2(int32_t location, core::SizeType count, bool transpose, const float *value);
extern void uniformMatrix3(int32_t location, core::SizeType count, bool transpose, const float *value);
extern void uniformMatrix4(int32_t location, core::SizeType count, bool transpose, const float *value);
extern void useProgram(uint32_t program);
extern void vertexAttrib1(uint32_t index, float x);
extern void vertexAttrib1(uint32_t index, const float *v);
extern void vertexAttrib2(uint32_t index, float x, float y);
extern void vertexAttrib2(uint32_t index, const float *v);
extern void vertexAttrib3(uint32_t index, float x, float y, float z);
extern void vertexAttrib3(uint32_t index, const float *v);
extern void vertexAttrib4(uint32_t index, float x, float y, float z, float w);
extern void vertexAttrib4(uint32_t index, const float *v);
extern void vertexAttribPointer(uint32_t index, int32_t size, VertexAttribPointerType type, bool normalized, core::SizeType stride, const void *pointer);
extern void viewport(int32_t x, int32_t y, core::SizeType width, core::SizeType height);
}
