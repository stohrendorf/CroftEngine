#pragma once
#include "soglb_core.hpp" // IWYU pragma: export
namespace gl::api
{
// API feature level: API_LEVEL_GL_VERSION_4_5_core

// special numbers
constexpr auto False = 0;
constexpr auto None = 0;
constexpr auto NoError = 0;
constexpr auto One = 1;
constexpr auto TimeoutIgnored = 0xFFFFFFFFFFFFFFFFull;
constexpr auto True = 1;
constexpr auto Zero = 0;
constexpr auto InvalidIndex = 0xFFFFFFFFu;

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

enum class AtomicCounterBufferPName : core::EnumType
{
  AtomicCounterBufferActiveAtomicCounters = 0x92C5,
  AtomicCounterBufferActiveAtomicCounterIndices = 0x92C6,
  AtomicCounterBufferBinding = 0x92C1,
  AtomicCounterBufferDataSize = 0x92C4,
  AtomicCounterBufferReferencedByFragmentShader = 0x92CB,
  AtomicCounterBufferReferencedByGeometryShader = 0x92CA,
  AtomicCounterBufferReferencedByTessControlShader = 0x92C8,
  AtomicCounterBufferReferencedByTessEvaluationShader = 0x92C9,
  AtomicCounterBufferReferencedByVertexShader = 0x92C7,
  AtomicCounterBufferReferencedByComputeShader = 0x90ED,
};

enum class AttribMask : core::EnumType
{
  ColorBufferBit = 0x00004000,
  DepthBufferBit = 0x00000100,
  StencilBufferBit = 0x00000400,
};
constexpr core::Bitfield<AttribMask> operator|(AttribMask left, AttribMask right)
{
  return core::Bitfield<AttribMask>(left) | right;
}

enum class AttributeType : core::EnumType
{
  Float = 0x1406,
  Int = 0x1404,
  UnsignedInt = 0x1405,
  Double = 0x140A,
  Bool = 0x8B56,
  BoolVec2 = 0x8B57,
  BoolVec3 = 0x8B58,
  BoolVec4 = 0x8B59,
  FloatMat2 = 0x8B5A,
  FloatMat3 = 0x8B5B,
  FloatMat4 = 0x8B5C,
  FloatVec2 = 0x8B50,
  FloatVec3 = 0x8B51,
  FloatVec4 = 0x8B52,
  IntVec2 = 0x8B53,
  IntVec3 = 0x8B54,
  IntVec4 = 0x8B55,
  Sampler1d = 0x8B5D,
  Sampler1dShadow = 0x8B61,
  Sampler2d = 0x8B5E,
  Sampler2dShadow = 0x8B62,
  Sampler3d = 0x8B5F,
  SamplerCube = 0x8B60,
  FloatMat2x3 = 0x8B65,
  FloatMat2x4 = 0x8B66,
  FloatMat3x2 = 0x8B67,
  FloatMat3x4 = 0x8B68,
  FloatMat4x2 = 0x8B69,
  FloatMat4x3 = 0x8B6A,
  IntSampler1d = 0x8DC9,
  IntSampler1dArray = 0x8DCE,
  IntSampler2d = 0x8DCA,
  IntSampler2dArray = 0x8DCF,
  IntSampler3d = 0x8DCB,
  IntSamplerCube = 0x8DCC,
  Sampler1dArrayShadow = 0x8DC3,
  Sampler2dArrayShadow = 0x8DC4,
  SamplerCubeShadow = 0x8DC5,
  UnsignedIntSampler1d = 0x8DD1,
  UnsignedIntSampler1dArray = 0x8DD6,
  UnsignedIntSampler2d = 0x8DD2,
  UnsignedIntSampler2dArray = 0x8DD7,
  UnsignedIntSampler3d = 0x8DD3,
  UnsignedIntSamplerCube = 0x8DD4,
  UnsignedIntVec2 = 0x8DC6,
  UnsignedIntVec3 = 0x8DC7,
  UnsignedIntVec4 = 0x8DC8,
  IntSampler2dRect = 0x8DCD,
  IntSamplerBuffer = 0x8DD0,
  Sampler2dRect = 0x8B63,
  Sampler2dRectShadow = 0x8B64,
  SamplerBuffer = 0x8DC2,
  UnsignedIntSampler2dRect = 0x8DD5,
  UnsignedIntSamplerBuffer = 0x8DD8,
  IntSampler2dMultisample = 0x9109,
  IntSampler2dMultisampleArray = 0x910C,
  Sampler2dMultisample = 0x9108,
  Sampler2dMultisampleArray = 0x910B,
  UnsignedIntSampler2dMultisample = 0x910A,
  UnsignedIntSampler2dMultisampleArray = 0x910D,
  DoubleMat2 = 0x8F46,
  DoubleMat2x3 = 0x8F49,
  DoubleMat2x4 = 0x8F4A,
  DoubleMat3 = 0x8F47,
  DoubleMat3x2 = 0x8F4B,
  DoubleMat3x4 = 0x8F4C,
  DoubleMat4 = 0x8F48,
  DoubleMat4x2 = 0x8F4D,
  DoubleMat4x3 = 0x8F4E,
  DoubleVec2 = 0x8FFC,
  DoubleVec3 = 0x8FFD,
  DoubleVec4 = 0x8FFE,
  IntSamplerCubeMapArray = 0x900E,
  SamplerCubeMapArray = 0x900C,
  SamplerCubeMapArrayShadow = 0x900D,
  UnsignedIntSamplerCubeMapArray = 0x900F,
  Image1d = 0x904C,
  Image1dArray = 0x9052,
  Image2d = 0x904D,
  Image2dArray = 0x9053,
  Image2dMultisample = 0x9055,
  Image2dMultisampleArray = 0x9056,
  Image2dRect = 0x904F,
  Image3d = 0x904E,
  ImageBuffer = 0x9051,
  ImageCube = 0x9050,
  ImageCubeMapArray = 0x9054,
  IntImage1d = 0x9057,
  IntImage1dArray = 0x905D,
  IntImage2d = 0x9058,
  IntImage2dArray = 0x905E,
  IntImage2dMultisample = 0x9060,
  IntImage2dMultisampleArray = 0x9061,
  IntImage2dRect = 0x905A,
  IntImage3d = 0x9059,
  IntImageBuffer = 0x905C,
  IntImageCube = 0x905B,
  IntImageCubeMapArray = 0x905F,
  UnsignedIntImage1d = 0x9062,
  UnsignedIntImage1dArray = 0x9068,
  UnsignedIntImage2d = 0x9063,
  UnsignedIntImage2dArray = 0x9069,
  UnsignedIntImage2dMultisample = 0x906B,
  UnsignedIntImage2dMultisampleArray = 0x906C,
  UnsignedIntImage2dRect = 0x9065,
  UnsignedIntImage3d = 0x9064,
  UnsignedIntImageBuffer = 0x9067,
  UnsignedIntImageCube = 0x9066,
  UnsignedIntImageCubeMapArray = 0x906A,
};

enum class BindTransformFeedbackTarget : core::EnumType
{
  TransformFeedback = 0x8E22,
};

enum class BinormalPointerTypeEXT : core::EnumType
{
  Byte = 0x1400,
  Float = 0x1406,
  Int = 0x1404,
  Short = 0x1402,
  Double = 0x140A,
};

enum class BlendEquationModeEXT : core::EnumType
{
  FuncAdd = 0x8006,
  FuncReverseSubtract = 0x800B,
  FuncSubtract = 0x800A,
  Max = 0x8008,
  Min = 0x8007,
};

enum class BlendingFactor : core::EnumType
{
  DstAlpha = 0x0304,
  DstColor = 0x0306,
  One = 1,
  OneMinusDstAlpha = 0x0305,
  OneMinusDstColor = 0x0307,
  OneMinusSrcAlpha = 0x0303,
  OneMinusSrcColor = 0x0301,
  SrcAlpha = 0x0302,
  SrcAlphaSaturate = 0x0308,
  SrcColor = 0x0300,
  Zero = 0,
  ConstantAlpha = 0x8003,
  ConstantColor = 0x8001,
  OneMinusConstantAlpha = 0x8004,
  OneMinusConstantColor = 0x8002,
  Src1Alpha = 0x8589,
  OneMinusSrc1Alpha = 0x88FB,
  OneMinusSrc1Color = 0x88FA,
  Src1Color = 0x88F9,
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

enum class Buffer : core::EnumType
{
  Color = 0x1800,
  Depth = 0x1801,
  Stencil = 0x1802,
};

enum class BufferAccess : core::EnumType
{
  ReadOnly = 0x88B8,
  ReadWrite = 0x88BA,
  WriteOnly = 0x88B9,
};

enum class BufferPName : core::EnumType
{
  BufferAccess = 0x88BB,
  BufferMapped = 0x88BC,
  BufferSize = 0x8764,
  BufferUsage = 0x8765,
  BufferAccessFlags = 0x911F,
  BufferMapLength = 0x9120,
  BufferMapOffset = 0x9121,
  BufferImmutableStorage = 0x821F,
  BufferStorageFlags = 0x8220,
};

enum class BufferPointerName : core::EnumType
{
  BufferMapPointer = 0x88BD,
};

enum class BufferStorageMask : core::EnumType
{
  MapReadBit = 0x0001,
  MapWriteBit = 0x0002,
  ClientStorageBit = 0x0200,
  DynamicStorageBit = 0x0100,
  MapCoherentBit = 0x0080,
  MapPersistentBit = 0x0040,
};
constexpr core::Bitfield<BufferStorageMask> operator|(BufferStorageMask left, BufferStorageMask right)
{
  return core::Bitfield<BufferStorageMask>(left) | right;
}

enum class BufferStorageTarget : core::EnumType
{
  ArrayBuffer = 0x8892,
  ElementArrayBuffer = 0x8893,
  PixelPackBuffer = 0x88EB,
  PixelUnpackBuffer = 0x88EC,
  TransformFeedbackBuffer = 0x8C8E,
  CopyReadBuffer = 0x8F36,
  CopyWriteBuffer = 0x8F37,
  TextureBuffer = 0x8C2A,
  UniformBuffer = 0x8A11,
  DrawIndirectBuffer = 0x8F3F,
  AtomicCounterBuffer = 0x92C0,
  DispatchIndirectBuffer = 0x90EE,
  ShaderStorageBuffer = 0x90D2,
  QueryBuffer = 0x9192,
};

enum class BufferTarget : core::EnumType
{
  ArrayBuffer = 0x8892,
  ElementArrayBuffer = 0x8893,
  PixelPackBuffer = 0x88EB,
  PixelUnpackBuffer = 0x88EC,
  TransformFeedbackBuffer = 0x8C8E,
  CopyReadBuffer = 0x8F36,
  CopyWriteBuffer = 0x8F37,
  TextureBuffer = 0x8C2A,
  UniformBuffer = 0x8A11,
  DrawIndirectBuffer = 0x8F3F,
  AtomicCounterBuffer = 0x92C0,
  DispatchIndirectBuffer = 0x90EE,
  ShaderStorageBuffer = 0x90D2,
  QueryBuffer = 0x9192,
};

enum class BufferUsage : core::EnumType
{
  DynamicCopy = 0x88EA,
  DynamicDraw = 0x88E8,
  DynamicRead = 0x88E9,
  StaticCopy = 0x88E6,
  StaticDraw = 0x88E4,
  StaticRead = 0x88E5,
  StreamCopy = 0x88E2,
  StreamDraw = 0x88E0,
  StreamRead = 0x88E1,
};

enum class ClampColorMode : core::EnumType
{
  False = 0,
  True = 1,
  FixedOnly = 0x891D,
};

enum class ClampColorTarget : core::EnumType
{
  ClampReadColor = 0x891C,
};

enum class ClearBufferMask : core::EnumType
{
  ColorBufferBit = 0x00004000,
  DepthBufferBit = 0x00000100,
  StencilBufferBit = 0x00000400,
};
constexpr core::Bitfield<ClearBufferMask> operator|(ClearBufferMask left, ClearBufferMask right)
{
  return core::Bitfield<ClearBufferMask>(left) | right;
}

enum class ClipControlDepth : core::EnumType
{
  NegativeOneToOne = 0x935E,
  ZeroToOne = 0x935F,
};

enum class ClipControlOrigin : core::EnumType
{
  LowerLeft = 0x8CA1,
  UpperLeft = 0x8CA2,
};

enum class ClipPlaneName : core::EnumType
{
  ClipDistance0 = 0x3000,
  ClipDistance1 = 0x3001,
  ClipDistance2 = 0x3002,
  ClipDistance3 = 0x3003,
  ClipDistance4 = 0x3004,
  ClipDistance5 = 0x3005,
  ClipDistance6 = 0x3006,
  ClipDistance7 = 0x3007,
};

enum class ColorBuffer : core::EnumType
{
  Back = 0x0405,
  BackLeft = 0x0402,
  BackRight = 0x0403,
  Front = 0x0404,
  FrontAndBack = 0x0408,
  FrontLeft = 0x0400,
  FrontRight = 0x0401,
  Left = 0x0406,
  None = 0,
  Right = 0x0407,
  ColorAttachment0 = 0x8CE0,
  ColorAttachment1 = 0x8CE1,
  ColorAttachment10 = 0x8CEA,
  ColorAttachment11 = 0x8CEB,
  ColorAttachment12 = 0x8CEC,
  ColorAttachment13 = 0x8CED,
  ColorAttachment14 = 0x8CEE,
  ColorAttachment15 = 0x8CEF,
  ColorAttachment16 = 0x8CF0,
  ColorAttachment17 = 0x8CF1,
  ColorAttachment18 = 0x8CF2,
  ColorAttachment19 = 0x8CF3,
  ColorAttachment2 = 0x8CE2,
  ColorAttachment20 = 0x8CF4,
  ColorAttachment21 = 0x8CF5,
  ColorAttachment22 = 0x8CF6,
  ColorAttachment23 = 0x8CF7,
  ColorAttachment24 = 0x8CF8,
  ColorAttachment25 = 0x8CF9,
  ColorAttachment26 = 0x8CFA,
  ColorAttachment27 = 0x8CFB,
  ColorAttachment28 = 0x8CFC,
  ColorAttachment29 = 0x8CFD,
  ColorAttachment3 = 0x8CE3,
  ColorAttachment30 = 0x8CFE,
  ColorAttachment31 = 0x8CFF,
  ColorAttachment4 = 0x8CE4,
  ColorAttachment5 = 0x8CE5,
  ColorAttachment6 = 0x8CE6,
  ColorAttachment7 = 0x8CE7,
  ColorAttachment8 = 0x8CE8,
  ColorAttachment9 = 0x8CE9,
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
  Double = 0x140A,
};

enum class CombinerBiasNV : core::EnumType
{
  None = 0,
};

enum class CombinerComponentUsageNV : core::EnumType
{
  Alpha = 0x1906,
  Blue = 0x1905,
  Rgb = 0x1907,
};

enum class CombinerPortionNV : core::EnumType
{
  Alpha = 0x1906,
  Rgb = 0x1907,
};

enum class CombinerScaleNV : core::EnumType
{
  None = 0,
};

enum class ConditionalRenderMode : core::EnumType
{
  QueryByRegionNoWait = 0x8E16,
  QueryByRegionWait = 0x8E15,
  QueryNoWait = 0x8E14,
  QueryWait = 0x8E13,
  QueryByRegionNoWaitInverted = 0x8E1A,
  QueryByRegionWaitInverted = 0x8E19,
  QueryNoWaitInverted = 0x8E18,
  QueryWaitInverted = 0x8E17,
};

enum class ContextFlagMask : core::EnumType
{
  ContextFlagForwardCompatibleBit = 0x00000001,
  ContextFlagDebugBit = 0x00000002,
  ContextFlagRobustAccessBit = 0x00000004,
};
constexpr core::Bitfield<ContextFlagMask> operator|(ContextFlagMask left, ContextFlagMask right)
{
  return core::Bitfield<ContextFlagMask>(left) | right;
}

enum class ContextProfileMask : core::EnumType
{
  ContextCompatibilityProfileBit = 0x00000002,
  ContextCoreProfileBit = 0x00000001,
};
constexpr core::Bitfield<ContextProfileMask> operator|(ContextProfileMask left, ContextProfileMask right)
{
  return core::Bitfield<ContextProfileMask>(left) | right;
}

enum class CopyBufferSubDataTarget : core::EnumType
{
  ArrayBuffer = 0x8892,
  ElementArrayBuffer = 0x8893,
  PixelPackBuffer = 0x88EB,
  PixelUnpackBuffer = 0x88EC,
  TransformFeedbackBuffer = 0x8C8E,
  CopyReadBuffer = 0x8F36,
  CopyWriteBuffer = 0x8F37,
  TextureBuffer = 0x8C2A,
  UniformBuffer = 0x8A11,
  DrawIndirectBuffer = 0x8F3F,
  AtomicCounterBuffer = 0x92C0,
  DispatchIndirectBuffer = 0x90EE,
  ShaderStorageBuffer = 0x90D2,
  QueryBuffer = 0x9192,
};

enum class CopyImageSubDataTarget : core::EnumType
{
  Texture1d = 0x0DE0,
  Texture2d = 0x0DE1,
  Texture3d = 0x806F,
  TextureCubeMap = 0x8513,
  Renderbuffer = 0x8D41,
  Texture1dArray = 0x8C18,
  Texture2dArray = 0x8C1A,
  TextureRectangle = 0x84F5,
  Texture2dMultisample = 0x9100,
  Texture2dMultisampleArray = 0x9102,
  TextureCubeMapArray = 0x9009,
};

enum class DebugSeverity : core::EnumType
{
  DontCare = 0x1100,
  DebugSeverityHigh = 0x9146,
  DebugSeverityLow = 0x9148,
  DebugSeverityMedium = 0x9147,
  DebugSeverityNotification = 0x826B,
};

enum class DebugSource : core::EnumType
{
  DontCare = 0x1100,
  DebugSourceApi = 0x8246,
  DebugSourceApplication = 0x824A,
  DebugSourceOther = 0x824B,
  DebugSourceShaderCompiler = 0x8248,
  DebugSourceThirdParty = 0x8249,
  DebugSourceWindowSystem = 0x8247,
};

enum class DebugType : core::EnumType
{
  DontCare = 0x1100,
  DebugTypeDeprecatedBehavior = 0x824D,
  DebugTypeError = 0x824C,
  DebugTypeMarker = 0x8268,
  DebugTypeOther = 0x8251,
  DebugTypePerformance = 0x8250,
  DebugTypePopGroup = 0x826A,
  DebugTypePortability = 0x824F,
  DebugTypePushGroup = 0x8269,
  DebugTypeUndefinedBehavior = 0x824E,
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

enum class DepthStencilTextureMode : core::EnumType
{
  DepthComponent = 0x1902,
  StencilIndex = 0x1901,
};

enum class DrawBufferMode : core::EnumType
{
  Back = 0x0405,
  BackLeft = 0x0402,
  BackRight = 0x0403,
  Front = 0x0404,
  FrontAndBack = 0x0408,
  FrontLeft = 0x0400,
  FrontRight = 0x0401,
  Left = 0x0406,
  None = 0,
  Right = 0x0407,
  ColorAttachment0 = 0x8CE0,
  ColorAttachment1 = 0x8CE1,
  ColorAttachment10 = 0x8CEA,
  ColorAttachment11 = 0x8CEB,
  ColorAttachment12 = 0x8CEC,
  ColorAttachment13 = 0x8CED,
  ColorAttachment14 = 0x8CEE,
  ColorAttachment15 = 0x8CEF,
  ColorAttachment16 = 0x8CF0,
  ColorAttachment17 = 0x8CF1,
  ColorAttachment18 = 0x8CF2,
  ColorAttachment19 = 0x8CF3,
  ColorAttachment2 = 0x8CE2,
  ColorAttachment20 = 0x8CF4,
  ColorAttachment21 = 0x8CF5,
  ColorAttachment22 = 0x8CF6,
  ColorAttachment23 = 0x8CF7,
  ColorAttachment24 = 0x8CF8,
  ColorAttachment25 = 0x8CF9,
  ColorAttachment26 = 0x8CFA,
  ColorAttachment27 = 0x8CFB,
  ColorAttachment28 = 0x8CFC,
  ColorAttachment29 = 0x8CFD,
  ColorAttachment3 = 0x8CE3,
  ColorAttachment30 = 0x8CFE,
  ColorAttachment31 = 0x8CFF,
  ColorAttachment4 = 0x8CE4,
  ColorAttachment5 = 0x8CE5,
  ColorAttachment6 = 0x8CE6,
  ColorAttachment7 = 0x8CE7,
  ColorAttachment8 = 0x8CE8,
  ColorAttachment9 = 0x8CE9,
};

enum class DrawElementsType : core::EnumType
{
  UnsignedByte = 0x1401,
  UnsignedInt = 0x1405,
  UnsignedShort = 0x1403,
};

enum class ElementPointerTypeATI : core::EnumType
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
  LineSmooth = 0x0B20,
  PolygonSmooth = 0x0B41,
  ScissorTest = 0x0C11,
  StencilTest = 0x0B90,
  Texture1d = 0x0DE0,
  Texture2d = 0x0DE1,
  ColorLogicOp = 0x0BF2,
  PolygonOffsetFill = 0x8037,
  PolygonOffsetLine = 0x2A02,
  PolygonOffsetPoint = 0x2A01,
  VertexArray = 0x8074,
  Multisample = 0x809D,
  SampleAlphaToCoverage = 0x809E,
  SampleAlphaToOne = 0x809F,
  SampleCoverage = 0x80A0,
  TextureCubeMap = 0x8513,
  ClipDistance0 = 0x3000,
  ClipDistance1 = 0x3001,
  ClipDistance2 = 0x3002,
  ClipDistance3 = 0x3003,
  ClipDistance4 = 0x3004,
  ClipDistance5 = 0x3005,
  ClipDistance6 = 0x3006,
  ClipDistance7 = 0x3007,
  FramebufferSrgb = 0x8DB9,
  RasterizerDiscard = 0x8C89,
  PrimitiveRestart = 0x8F9D,
  TextureRectangle = 0x84F5,
  DepthClamp = 0x864F,
  ProgramPointSize = 0x8642,
  SampleMask = 0x8E51,
  TextureCubeMapSeamless = 0x884F,
  SampleShading = 0x8C36,
  DebugOutput = 0x92E0,
  DebugOutputSynchronous = 0x8242,
  PrimitiveRestartFixedIndex = 0x8D69,
};

enum class ErrorCode : core::EnumType
{
  InvalidEnum = 0x0500,
  InvalidOperation = 0x0502,
  InvalidValue = 0x0501,
  NoError = 0,
  OutOfMemory = 0x0505,
  StackOverflow = 0x0503,
  StackUnderflow = 0x0504,
  InvalidFramebufferOperation = 0x0506,
};

enum class FogCoordSrc : core::EnumType
{
};

enum class FogCoordinatePointerType : core::EnumType
{
  Float = 0x1406,
  Double = 0x140A,
};

enum class FogMode : core::EnumType
{
  Linear = 0x2601,
};

enum class FogPName : core::EnumType
{
};

enum class FogPointerTypeEXT : core::EnumType
{
  Float = 0x1406,
  Double = 0x140A,
};

enum class FogPointerTypeIBM : core::EnumType
{
  Float = 0x1406,
  Double = 0x140A,
};

enum class FragmentShaderDestMaskATI : core::EnumType
{
  None = 0,
};
constexpr core::Bitfield<FragmentShaderDestMaskATI> operator|(FragmentShaderDestMaskATI left,
                                                              FragmentShaderDestMaskATI right)
{
  return core::Bitfield<FragmentShaderDestMaskATI>(left) | right;
}

enum class FragmentShaderDestModMaskATI : core::EnumType
{
  None = 0,
};
constexpr core::Bitfield<FragmentShaderDestModMaskATI> operator|(FragmentShaderDestModMaskATI left,
                                                                 FragmentShaderDestModMaskATI right)
{
  return core::Bitfield<FragmentShaderDestModMaskATI>(left) | right;
}

enum class FragmentShaderGenericSourceATI : core::EnumType
{
  One = 1,
  Zero = 0,
};

enum class FragmentShaderTextureSourceATI : core::EnumType
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

enum class FragmentShaderValueRepATI : core::EnumType
{
  Alpha = 0x1906,
  Blue = 0x1905,
  Green = 0x1904,
  None = 0,
  Red = 0x1903,
};

enum class FramebufferAttachment : core::EnumType
{
  ColorAttachment0 = 0x8CE0,
  ColorAttachment1 = 0x8CE1,
  ColorAttachment10 = 0x8CEA,
  ColorAttachment11 = 0x8CEB,
  ColorAttachment12 = 0x8CEC,
  ColorAttachment13 = 0x8CED,
  ColorAttachment14 = 0x8CEE,
  ColorAttachment15 = 0x8CEF,
  ColorAttachment16 = 0x8CF0,
  ColorAttachment17 = 0x8CF1,
  ColorAttachment18 = 0x8CF2,
  ColorAttachment19 = 0x8CF3,
  ColorAttachment2 = 0x8CE2,
  ColorAttachment20 = 0x8CF4,
  ColorAttachment21 = 0x8CF5,
  ColorAttachment22 = 0x8CF6,
  ColorAttachment23 = 0x8CF7,
  ColorAttachment24 = 0x8CF8,
  ColorAttachment25 = 0x8CF9,
  ColorAttachment26 = 0x8CFA,
  ColorAttachment27 = 0x8CFB,
  ColorAttachment28 = 0x8CFC,
  ColorAttachment29 = 0x8CFD,
  ColorAttachment3 = 0x8CE3,
  ColorAttachment30 = 0x8CFE,
  ColorAttachment31 = 0x8CFF,
  ColorAttachment4 = 0x8CE4,
  ColorAttachment5 = 0x8CE5,
  ColorAttachment6 = 0x8CE6,
  ColorAttachment7 = 0x8CE7,
  ColorAttachment8 = 0x8CE8,
  ColorAttachment9 = 0x8CE9,
  DepthAttachment = 0x8D00,
  DepthStencilAttachment = 0x821A,
  StencilAttachment = 0x8D20,
};

enum class FramebufferAttachmentParameterName : core::EnumType
{
  FramebufferAttachmentAlphaSize = 0x8215,
  FramebufferAttachmentBlueSize = 0x8214,
  FramebufferAttachmentColorEncoding = 0x8210,
  FramebufferAttachmentComponentType = 0x8211,
  FramebufferAttachmentDepthSize = 0x8216,
  FramebufferAttachmentGreenSize = 0x8213,
  FramebufferAttachmentObjectName = 0x8CD1,
  FramebufferAttachmentObjectType = 0x8CD0,
  FramebufferAttachmentRedSize = 0x8212,
  FramebufferAttachmentStencilSize = 0x8217,
  FramebufferAttachmentTextureCubeMapFace = 0x8CD3,
  FramebufferAttachmentTextureLayer = 0x8CD4,
  FramebufferAttachmentTextureLevel = 0x8CD2,
  FramebufferAttachmentLayered = 0x8DA7,
};

enum class FramebufferParameterName : core::EnumType
{
  FramebufferDefaultFixedSampleLocations = 0x9314,
  FramebufferDefaultHeight = 0x9311,
  FramebufferDefaultLayers = 0x9312,
  FramebufferDefaultSamples = 0x9313,
  FramebufferDefaultWidth = 0x9310,
};

enum class FramebufferStatus : core::EnumType
{
  FramebufferComplete = 0x8CD5,
  FramebufferIncompleteAttachment = 0x8CD6,
  FramebufferIncompleteDrawBuffer = 0x8CDB,
  FramebufferIncompleteMissingAttachment = 0x8CD7,
  FramebufferIncompleteMultisample = 0x8D56,
  FramebufferIncompleteReadBuffer = 0x8CDC,
  FramebufferUndefined = 0x8219,
  FramebufferUnsupported = 0x8CDD,
  FramebufferIncompleteLayerTargets = 0x8DA8,
};

enum class FramebufferTarget : core::EnumType
{
  DrawFramebuffer = 0x8CA9,
  Framebuffer = 0x8D40,
  ReadFramebuffer = 0x8CA8,
};

enum class FrontFaceDirection : core::EnumType
{
  Ccw = 0x0901,
  Cw = 0x0900,
};

enum class GetFramebufferParameter : core::EnumType
{
  Doublebuffer = 0x0C32,
  Stereo = 0x0C33,
  Samples = 0x80A9,
  SampleBuffers = 0x80A8,
  ImplementationColorReadFormat = 0x8B9B,
  ImplementationColorReadType = 0x8B9A,
  FramebufferDefaultFixedSampleLocations = 0x9314,
  FramebufferDefaultHeight = 0x9311,
  FramebufferDefaultLayers = 0x9312,
  FramebufferDefaultSamples = 0x9313,
  FramebufferDefaultWidth = 0x9310,
};

enum class GetMultisamplePNameNV : core::EnumType
{
  SamplePosition = 0x8E50,
};

enum class GetPName : core::EnumType
{
  Blend = 0x0BE2,
  BlendDst = 0x0BE0,
  BlendSrc = 0x0BE1,
  ColorClearValue = 0x0C22,
  ColorWritemask = 0x0C23,
  CullFace = 0x0B44,
  CullFaceMode = 0x0B45,
  DepthClearValue = 0x0B73,
  DepthFunc = 0x0B74,
  DepthRange = 0x0B70,
  DepthTest = 0x0B71,
  DepthWritemask = 0x0B72,
  Dither = 0x0BD0,
  Doublebuffer = 0x0C32,
  DrawBuffer = 0x0C01,
  FrontFace = 0x0B46,
  LineSmooth = 0x0B20,
  LineSmoothHint = 0x0C52,
  LineWidth = 0x0B21,
  LineWidthGranularity = 0x0B23,
  LineWidthRange = 0x0B22,
  LogicOpMode = 0x0BF0,
  MaxServerWaitTimeout = 0x9111,
  MaxTextureSize = 0x0D33,
  MaxViewportDims = 0x0D3A,
  PackAlignment = 0x0D05,
  PackLsbFirst = 0x0D01,
  PackRowLength = 0x0D02,
  PackSkipPixels = 0x0D04,
  PackSkipRows = 0x0D03,
  PackSwapBytes = 0x0D00,
  PointSize = 0x0B11,
  PointSizeGranularity = 0x0B13,
  PointSizeRange = 0x0B12,
  PolygonMode = 0x0B40,
  PolygonSmooth = 0x0B41,
  PolygonSmoothHint = 0x0C53,
  ReadBuffer = 0x0C02,
  ScissorBox = 0x0C10,
  ScissorTest = 0x0C11,
  StencilClearValue = 0x0B91,
  StencilFail = 0x0B94,
  StencilFunc = 0x0B92,
  StencilPassDepthFail = 0x0B95,
  StencilPassDepthPass = 0x0B96,
  StencilRef = 0x0B97,
  StencilTest = 0x0B90,
  StencilValueMask = 0x0B93,
  StencilWritemask = 0x0B98,
  Stereo = 0x0C33,
  SubpixelBits = 0x0D50,
  Texture1d = 0x0DE0,
  Texture2d = 0x0DE1,
  UnpackAlignment = 0x0CF5,
  UnpackLsbFirst = 0x0CF1,
  UnpackRowLength = 0x0CF2,
  UnpackSkipPixels = 0x0CF4,
  UnpackSkipRows = 0x0CF3,
  UnpackSwapBytes = 0x0CF0,
  Viewport = 0x0BA2,
  ColorLogicOp = 0x0BF2,
  PolygonOffsetFactor = 0x8038,
  PolygonOffsetFill = 0x8037,
  PolygonOffsetLine = 0x2A02,
  PolygonOffsetPoint = 0x2A01,
  PolygonOffsetUnits = 0x2A00,
  TextureBinding1d = 0x8068,
  TextureBinding2d = 0x8069,
  VertexArray = 0x8074,
  AliasedLineWidthRange = 0x846E,
  Max3dTextureSize = 0x8073,
  MaxElementsIndices = 0x80E9,
  MaxElementsVertices = 0x80E8,
  PackImageHeight = 0x806C,
  PackSkipImages = 0x806B,
  SmoothLineWidthGranularity = 0x0B23,
  SmoothLineWidthRange = 0x0B22,
  SmoothPointSizeGranularity = 0x0B13,
  SmoothPointSizeRange = 0x0B12,
  TextureBinding3d = 0x806A,
  UnpackImageHeight = 0x806E,
  UnpackSkipImages = 0x806D,
  ActiveTexture = 0x84E0,
  CompressedTextureFormats = 0x86A3,
  MaxCubeMapTextureSize = 0x851C,
  NumCompressedTextureFormats = 0x86A2,
  Samples = 0x80A9,
  SampleBuffers = 0x80A8,
  SampleCoverageInvert = 0x80AB,
  SampleCoverageValue = 0x80AA,
  TextureBindingCubeMap = 0x8514,
  TextureCompressionHint = 0x84EF,
  BlendColor = 0x8005,
  BlendDstAlpha = 0x80CA,
  BlendDstRgb = 0x80C8,
  BlendEquation = 0x8009,
  BlendSrcAlpha = 0x80CB,
  BlendSrcRgb = 0x80C9,
  MaxTextureLodBias = 0x84FD,
  PointFadeThresholdSize = 0x8128,
  ArrayBufferBinding = 0x8894,
  ElementArrayBufferBinding = 0x8895,
  BlendEquationAlpha = 0x883D,
  BlendEquationRgb = 0x8009,
  CurrentProgram = 0x8B8D,
  FragmentShaderDerivativeHint = 0x8B8B,
  MaxCombinedTextureImageUnits = 0x8B4D,
  MaxDrawBuffers = 0x8824,
  MaxFragmentUniformComponents = 0x8B49,
  MaxTextureImageUnits = 0x8872,
  MaxVaryingFloats = 0x8B4B,
  MaxVertexAttribs = 0x8869,
  MaxVertexTextureImageUnits = 0x8B4C,
  MaxVertexUniformComponents = 0x8B4A,
  StencilBackFail = 0x8801,
  StencilBackFunc = 0x8800,
  StencilBackPassDepthFail = 0x8802,
  StencilBackPassDepthPass = 0x8803,
  StencilBackRef = 0x8CA3,
  StencilBackValueMask = 0x8CA4,
  StencilBackWritemask = 0x8CA5,
  PixelPackBufferBinding = 0x88ED,
  PixelUnpackBufferBinding = 0x88EF,
  ContextFlags = 0x821E,
  DrawFramebufferBinding = 0x8CA6,
  MajorVersion = 0x821B,
  MaxArrayTextureLayers = 0x88FF,
  MaxClipDistances = 0x0D32,
  MaxColorAttachments = 0x8CDF,
  MaxProgramTexelOffset = 0x8905,
  MaxRenderbufferSize = 0x84E8,
  MaxVaryingComponents = 0x8B4B,
  MinorVersion = 0x821C,
  MinProgramTexelOffset = 0x8904,
  NumExtensions = 0x821D,
  ReadFramebufferBinding = 0x8CAA,
  RenderbufferBinding = 0x8CA7,
  TextureBinding1dArray = 0x8C1C,
  TextureBinding2dArray = 0x8C1D,
  TransformFeedbackBufferBinding = 0x8C8F,
  TransformFeedbackBufferSize = 0x8C85,
  TransformFeedbackBufferStart = 0x8C84,
  VertexArrayBinding = 0x85B5,
  MaxCombinedFragmentUniformComponents = 0x8A33,
  MaxCombinedGeometryUniformComponents = 0x8A32,
  MaxCombinedUniformBlocks = 0x8A2E,
  MaxCombinedVertexUniformComponents = 0x8A31,
  MaxFragmentUniformBlocks = 0x8A2D,
  MaxGeometryUniformBlocks = 0x8A2C,
  MaxRectangleTextureSize = 0x84F8,
  MaxTextureBufferSize = 0x8C2B,
  MaxUniformBlockSize = 0x8A30,
  MaxUniformBufferBindings = 0x8A2F,
  MaxVertexUniformBlocks = 0x8A2B,
  PrimitiveRestartIndex = 0x8F9E,
  TextureBindingBuffer = 0x8C2C,
  TextureBindingRectangle = 0x84F6,
  UniformBufferBinding = 0x8A28,
  UniformBufferOffsetAlignment = 0x8A34,
  UniformBufferSize = 0x8A2A,
  UniformBufferStart = 0x8A29,
  ContextProfileMask = 0x9126,
  MaxColorTextureSamples = 0x910E,
  MaxDepthTextureSamples = 0x910F,
  MaxFragmentInputComponents = 0x9125,
  MaxGeometryInputComponents = 0x9123,
  MaxGeometryOutputComponents = 0x9124,
  MaxGeometryTextureImageUnits = 0x8C29,
  MaxGeometryUniformComponents = 0x8DDF,
  MaxIntegerSamples = 0x9110,
  MaxSampleMaskWords = 0x8E59,
  MaxVertexOutputComponents = 0x9122,
  ProgramPointSize = 0x8642,
  ProvokingVertex = 0x8E4F,
  TextureBinding2dMultisample = 0x9104,
  TextureBinding2dMultisampleArray = 0x9105,
  MaxDualSourceDrawBuffers = 0x88FC,
  SamplerBinding = 0x8919,
  Timestamp = 0x8E28,
  MaxTessControlUniformBlocks = 0x8E89,
  MaxTessEvaluationUniformBlocks = 0x8E8A,
  ImplementationColorReadFormat = 0x8B9B,
  ImplementationColorReadType = 0x8B9A,
  LayerProvokingVertex = 0x825E,
  MaxFragmentUniformVectors = 0x8DFD,
  MaxVaryingVectors = 0x8DFC,
  MaxVertexUniformVectors = 0x8DFB,
  MaxViewports = 0x825B,
  NumProgramBinaryFormats = 0x87FE,
  NumShaderBinaryFormats = 0x8DF9,
  ProgramBinaryFormats = 0x87FF,
  ProgramPipelineBinding = 0x825A,
  ShaderBinaryFormats = 0x8DF8,
  ShaderCompiler = 0x8DFA,
  ViewportBoundsRange = 0x825D,
  ViewportIndexProvokingVertex = 0x825F,
  ViewportSubpixelBits = 0x825C,
  MaxCombinedAtomicCounters = 0x92D7,
  MaxFragmentAtomicCounters = 0x92D6,
  MaxGeometryAtomicCounters = 0x92D5,
  MaxTessControlAtomicCounters = 0x92D3,
  MaxTessEvaluationAtomicCounters = 0x92D4,
  MaxVertexAtomicCounters = 0x92D2,
  MinMapBufferAlignment = 0x90BC,
  DebugGroupStackDepth = 0x826D,
  DispatchIndirectBufferBinding = 0x90EF,
  MaxCombinedComputeUniformComponents = 0x8266,
  MaxCombinedShaderStorageBlocks = 0x90DC,
  MaxComputeAtomicCounters = 0x8265,
  MaxComputeAtomicCounterBuffers = 0x8264,
  MaxComputeShaderStorageBlocks = 0x90DB,
  MaxComputeTextureImageUnits = 0x91BC,
  MaxComputeUniformBlocks = 0x91BB,
  MaxComputeUniformComponents = 0x8263,
  MaxComputeWorkGroupCount = 0x91BE,
  MaxComputeWorkGroupInvocations = 0x90EB,
  MaxComputeWorkGroupSize = 0x91BF,
  MaxDebugGroupStackDepth = 0x826C,
  MaxElementIndex = 0x8D6B,
  MaxFragmentShaderStorageBlocks = 0x90DA,
  MaxFramebufferHeight = 0x9316,
  MaxFramebufferLayers = 0x9317,
  MaxFramebufferSamples = 0x9318,
  MaxFramebufferWidth = 0x9315,
  MaxGeometryShaderStorageBlocks = 0x90D7,
  MaxLabelLength = 0x82E8,
  MaxShaderStorageBufferBindings = 0x90DD,
  MaxTessControlShaderStorageBlocks = 0x90D8,
  MaxTessEvaluationShaderStorageBlocks = 0x90D9,
  MaxUniformLocations = 0x826E,
  MaxVertexAttribBindings = 0x82DA,
  MaxVertexAttribRelativeOffset = 0x82D9,
  MaxVertexShaderStorageBlocks = 0x90D6,
  ShaderStorageBufferBinding = 0x90D3,
  ShaderStorageBufferOffsetAlignment = 0x90DF,
  ShaderStorageBufferSize = 0x90D5,
  ShaderStorageBufferStart = 0x90D4,
  TextureBufferOffsetAlignment = 0x919F,
  VertexBindingDivisor = 0x82D6,
  VertexBindingOffset = 0x82D7,
  VertexBindingStride = 0x82D8,
};

enum class GetPointervPName : core::EnumType
{
  DebugCallbackFunction = 0x8244,
  DebugCallbackUserParam = 0x8245,
};

enum class GetTextureParameter : core::EnumType
{
  TextureBorderColor = 0x1004,
  TextureHeight = 0x1001,
  TextureMagFilter = 0x2800,
  TextureMinFilter = 0x2801,
  TextureWidth = 0x1000,
  TextureWrapS = 0x2802,
  TextureWrapT = 0x2803,
  TextureAlphaSize = 0x805F,
  TextureBlueSize = 0x805E,
  TextureGreenSize = 0x805D,
  TextureInternalFormat = 0x1003,
  TextureRedSize = 0x805C,
};

enum class GraphicsResetStatus : core::EnumType
{
  NoError = 0,
  GuiltyContextReset = 0x8253,
  InnocentContextReset = 0x8254,
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
  LineSmoothHint = 0x0C52,
  PolygonSmoothHint = 0x0C53,
  TextureCompressionHint = 0x84EF,
  FragmentShaderDerivativeHint = 0x8B8B,
  ProgramBinaryRetrievableHint = 0x8257,
};

enum class IndexFunctionEXT : core::EnumType
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

enum class IndexPointerType : core::EnumType
{
  Float = 0x1406,
  Int = 0x1404,
  Short = 0x1402,
  Double = 0x140A,
};

enum class InternalFormat : core::EnumType
{
  DepthComponent = 0x1902,
  Red = 0x1903,
  Rgb = 0x1907,
  Rgba = 0x1908,
  StencilIndex = 0x1901,
  R3G3B2 = 0x2A10,
  Rgb10 = 0x8052,
  Rgb10A2 = 0x8059,
  Rgb12 = 0x8053,
  Rgb16 = 0x8054,
  Rgb4 = 0x804F,
  Rgb5 = 0x8050,
  Rgb5A1 = 0x8057,
  Rgb8 = 0x8051,
  Rgba12 = 0x805A,
  Rgba16 = 0x805B,
  Rgba2 = 0x8055,
  Rgba4 = 0x8056,
  Rgba8 = 0x8058,
  CompressedRgb = 0x84ED,
  CompressedRgba = 0x84EE,
  DepthComponent16 = 0x81A5,
  DepthComponent24 = 0x81A6,
  DepthComponent32 = 0x81A7,
  CompressedSrgb = 0x8C48,
  CompressedSrgbAlpha = 0x8C49,
  Srgb = 0x8C40,
  Srgb8 = 0x8C41,
  Srgb8Alpha8 = 0x8C43,
  SrgbAlpha = 0x8C42,
  CompressedRed = 0x8225,
  CompressedRedRgtc1 = 0x8DBB,
  CompressedRg = 0x8226,
  CompressedRgRgtc2 = 0x8DBD,
  CompressedSignedRedRgtc1 = 0x8DBC,
  CompressedSignedRgRgtc2 = 0x8DBE,
  Depth24Stencil8 = 0x88F0,
  Depth32fStencil8 = 0x8CAD,
  DepthComponent32f = 0x8CAC,
  DepthStencil = 0x84F9,
  R11fG11fB10f = 0x8C3A,
  R16 = 0x822A,
  R16f = 0x822D,
  R16i = 0x8233,
  R16ui = 0x8234,
  R32f = 0x822E,
  R32i = 0x8235,
  R32ui = 0x8236,
  R8 = 0x8229,
  R8i = 0x8231,
  R8ui = 0x8232,
  Rg = 0x8227,
  Rg16 = 0x822C,
  Rg16f = 0x822F,
  Rg16i = 0x8239,
  Rg16ui = 0x823A,
  Rg32f = 0x8230,
  Rg32i = 0x823B,
  Rg32ui = 0x823C,
  Rg8 = 0x822B,
  Rg8i = 0x8237,
  Rg8ui = 0x8238,
  Rgb16f = 0x881B,
  Rgb16i = 0x8D89,
  Rgb16ui = 0x8D77,
  Rgb32f = 0x8815,
  Rgb32i = 0x8D83,
  Rgb32ui = 0x8D71,
  Rgb8i = 0x8D8F,
  Rgb8ui = 0x8D7D,
  Rgb9E5 = 0x8C3D,
  Rgba16f = 0x881A,
  Rgba16i = 0x8D88,
  Rgba16ui = 0x8D76,
  Rgba32f = 0x8814,
  Rgba32i = 0x8D82,
  Rgba32ui = 0x8D70,
  Rgba8i = 0x8D8E,
  Rgba8ui = 0x8D7C,
  StencilIndex1 = 0x8D46,
  StencilIndex16 = 0x8D49,
  StencilIndex4 = 0x8D47,
  StencilIndex8 = 0x8D48,
  R16Snorm = 0x8F98,
  R8Snorm = 0x8F94,
  Rg16Snorm = 0x8F99,
  Rg8Snorm = 0x8F95,
  Rgb16Snorm = 0x8F9A,
  Rgb8Snorm = 0x8F96,
  Rgba16Snorm = 0x8F9B,
  Rgba8Snorm = 0x8F97,
  Rgb10A2ui = 0x906F,
  Rgb565 = 0x8D62,
  CompressedRgbaBptcUnorm = 0x8E8C,
  CompressedRgbBptcSignedFloat = 0x8E8E,
  CompressedRgbBptcUnsignedFloat = 0x8E8F,
  CompressedSrgbAlphaBptcUnorm = 0x8E8D,
  CompressedR11Eac = 0x9270,
  CompressedRg11Eac = 0x9272,
  CompressedRgb8Etc2 = 0x9274,
  CompressedRgb8PunchthroughAlpha1Etc2 = 0x9276,
  CompressedRgba8Etc2Eac = 0x9278,
  CompressedSignedR11Eac = 0x9271,
  CompressedSignedRg11Eac = 0x9273,
  CompressedSrgb8Alpha8Etc2Eac = 0x9279,
  CompressedSrgb8Etc2 = 0x9275,
  CompressedSrgb8PunchthroughAlpha1Etc2 = 0x9277,
};

enum class InternalFormatPName : core::EnumType
{
  Samples = 0x80A9,
  TextureCompressed = 0x86A1,
  ImageFormatCompatibilityType = 0x90C7,
  NumSampleCounts = 0x9380,
  AutoGenerateMipmap = 0x8295,
  ClearBuffer = 0x82B4,
  ColorComponents = 0x8283,
  ColorEncoding = 0x8296,
  ColorRenderable = 0x8286,
  ComputeTexture = 0x82A0,
  DepthRenderable = 0x8287,
  Filter = 0x829A,
  FragmentTexture = 0x829F,
  FramebufferBlend = 0x828B,
  FramebufferRenderable = 0x8289,
  FramebufferRenderableLayered = 0x828A,
  GeometryTexture = 0x829E,
  GetTextureImageFormat = 0x8291,
  GetTextureImageType = 0x8292,
  ImageCompatibilityClass = 0x82A8,
  ImagePixelFormat = 0x82A9,
  ImagePixelType = 0x82AA,
  ImageTexelSize = 0x82A7,
  InternalformatAlphaSize = 0x8274,
  InternalformatAlphaType = 0x827B,
  InternalformatBlueSize = 0x8273,
  InternalformatBlueType = 0x827A,
  InternalformatDepthSize = 0x8275,
  InternalformatDepthType = 0x827C,
  InternalformatGreenSize = 0x8272,
  InternalformatGreenType = 0x8279,
  InternalformatPreferred = 0x8270,
  InternalformatRedSize = 0x8271,
  InternalformatRedType = 0x8278,
  InternalformatSharedSize = 0x8277,
  InternalformatStencilSize = 0x8276,
  InternalformatStencilType = 0x827D,
  InternalformatSupported = 0x826F,
  MaxDepth = 0x8280,
  MaxHeight = 0x827F,
  MaxLayers = 0x8281,
  MaxWidth = 0x827E,
  Mipmap = 0x8293,
  ReadPixels = 0x828C,
  ReadPixelsFormat = 0x828D,
  ReadPixelsType = 0x828E,
  ShaderImageAtomic = 0x82A6,
  ShaderImageLoad = 0x82A4,
  ShaderImageStore = 0x82A5,
  SimultaneousTextureAndDepthTest = 0x82AC,
  SimultaneousTextureAndDepthWrite = 0x82AE,
  SimultaneousTextureAndStencilTest = 0x82AD,
  SimultaneousTextureAndStencilWrite = 0x82AF,
  SrgbRead = 0x8297,
  SrgbWrite = 0x8298,
  StencilRenderable = 0x8288,
  TessControlTexture = 0x829C,
  TessEvaluationTexture = 0x829D,
  TextureCompressedBlockHeight = 0x82B2,
  TextureCompressedBlockSize = 0x82B3,
  TextureCompressedBlockWidth = 0x82B1,
  TextureGather = 0x82A2,
  TextureGatherShadow = 0x82A3,
  TextureImageFormat = 0x828F,
  TextureImageType = 0x8290,
  TextureShadow = 0x82A1,
  TextureView = 0x82B5,
  VertexTexture = 0x829B,
  ViewCompatibilityClass = 0x82B6,
  ClearTexture = 0x9365,
};

enum class InvalidateFramebufferAttachment : core::EnumType
{
  Color = 0x1800,
  Depth = 0x1801,
  Stencil = 0x1802,
  ColorAttachment0 = 0x8CE0,
  ColorAttachment1 = 0x8CE1,
  ColorAttachment10 = 0x8CEA,
  ColorAttachment11 = 0x8CEB,
  ColorAttachment12 = 0x8CEC,
  ColorAttachment13 = 0x8CED,
  ColorAttachment14 = 0x8CEE,
  ColorAttachment15 = 0x8CEF,
  ColorAttachment16 = 0x8CF0,
  ColorAttachment17 = 0x8CF1,
  ColorAttachment18 = 0x8CF2,
  ColorAttachment19 = 0x8CF3,
  ColorAttachment2 = 0x8CE2,
  ColorAttachment20 = 0x8CF4,
  ColorAttachment21 = 0x8CF5,
  ColorAttachment22 = 0x8CF6,
  ColorAttachment23 = 0x8CF7,
  ColorAttachment24 = 0x8CF8,
  ColorAttachment25 = 0x8CF9,
  ColorAttachment26 = 0x8CFA,
  ColorAttachment27 = 0x8CFB,
  ColorAttachment28 = 0x8CFC,
  ColorAttachment29 = 0x8CFD,
  ColorAttachment3 = 0x8CE3,
  ColorAttachment30 = 0x8CFE,
  ColorAttachment31 = 0x8CFF,
  ColorAttachment4 = 0x8CE4,
  ColorAttachment5 = 0x8CE5,
  ColorAttachment6 = 0x8CE6,
  ColorAttachment7 = 0x8CE7,
  ColorAttachment8 = 0x8CE8,
  ColorAttachment9 = 0x8CE9,
  DepthAttachment = 0x8D00,
  DepthStencilAttachment = 0x821A,
};

enum class LightEnvModeSGIX : core::EnumType
{
  Replace = 0x1E01,
};

enum class LightModelParameter : core::EnumType
{
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
};

enum class MapBufferAccessMask : core::EnumType
{
  MapFlushExplicitBit = 0x0010,
  MapInvalidateBufferBit = 0x0008,
  MapInvalidateRangeBit = 0x0004,
  MapReadBit = 0x0001,
  MapUnsynchronizedBit = 0x0020,
  MapWriteBit = 0x0002,
  MapCoherentBit = 0x0080,
  MapPersistentBit = 0x0040,
};
constexpr core::Bitfield<MapBufferAccessMask> operator|(MapBufferAccessMask left, MapBufferAccessMask right)
{
  return core::Bitfield<MapBufferAccessMask>(left) | right;
}

enum class MapTypeNV : core::EnumType
{
  Float = 0x1406,
  Double = 0x140A,
};

enum class MatrixIndexPointerType : core::EnumType
{
  UnsignedByte = 0x1401,
  UnsignedInt = 0x1405,
  UnsignedShort = 0x1403,
};

enum class MatrixMode : core::EnumType
{
  Texture = 0x1702,
};

enum class MemoryBarrierMask : core::EnumType
{
  AllBarrierBits = 0xFFFFFFFF,
  AtomicCounterBarrierBit = 0x00001000,
  BufferUpdateBarrierBit = 0x00000200,
  CommandBarrierBit = 0x00000040,
  ElementArrayBarrierBit = 0x00000002,
  FramebufferBarrierBit = 0x00000400,
  PixelBufferBarrierBit = 0x00000080,
  ShaderImageAccessBarrierBit = 0x00000020,
  TextureFetchBarrierBit = 0x00000008,
  TextureUpdateBarrierBit = 0x00000100,
  TransformFeedbackBarrierBit = 0x00000800,
  UniformBarrierBit = 0x00000004,
  VertexAttribArrayBarrierBit = 0x00000001,
  ShaderStorageBarrierBit = 0x00002000,
  ClientMappedBufferBarrierBit = 0x00004000,
  QueryBufferBarrierBit = 0x00008000,
};
constexpr core::Bitfield<MemoryBarrierMask> operator|(MemoryBarrierMask left, MemoryBarrierMask right)
{
  return core::Bitfield<MemoryBarrierMask>(left) | right;
}

enum class MeshMode1 : core::EnumType
{
  Line = 0x1B01,
  Point = 0x1B00,
};

enum class MeshMode2 : core::EnumType
{
  Fill = 0x1B02,
  Line = 0x1B01,
  Point = 0x1B00,
};

enum class NormalPointerType : core::EnumType
{
  Byte = 0x1400,
  Float = 0x1406,
  Int = 0x1404,
  Short = 0x1402,
  Double = 0x140A,
};

enum class ObjectIdentifier : core::EnumType
{
  Texture = 0x1702,
  VertexArray = 0x8074,
  Framebuffer = 0x8D40,
  Renderbuffer = 0x8D41,
  TransformFeedback = 0x8E22,
  Buffer = 0x82E0,
  Program = 0x82E2,
  ProgramPipeline = 0x82E4,
  Query = 0x82E3,
  Sampler = 0x82E6,
  Shader = 0x82E1,
};

enum class PatchParameterName : core::EnumType
{
  PatchDefaultInnerLevel = 0x8E73,
  PatchDefaultOuterLevel = 0x8E74,
  PatchVertices = 0x8E72,
};

enum class PathColorFormat : core::EnumType
{
  Alpha = 0x1906,
  None = 0,
  Rgb = 0x1907,
  Rgba = 0x1908,
};

enum class PathFillMode : core::EnumType
{
  Invert = 0x150A,
};

enum class PathFontStyle : core::EnumType
{
  None = 0,
};
constexpr core::Bitfield<PathFontStyle> operator|(PathFontStyle left, PathFontStyle right)
{
  return core::Bitfield<PathFontStyle>(left) | right;
}

enum class PathGenMode : core::EnumType
{
  None = 0,
};

enum class PathTransformType : core::EnumType
{
  None = 0,
};

enum class PipelineParameterName : core::EnumType
{
  FragmentShader = 0x8B30,
  InfoLogLength = 0x8B84,
  VertexShader = 0x8B31,
  GeometryShader = 0x8DD9,
  TessControlShader = 0x8E88,
  TessEvaluationShader = 0x8E87,
  ActiveProgram = 0x8259,
};

enum class PixelCopyType : core::EnumType
{
  Color = 0x1800,
  Depth = 0x1801,
  Stencil = 0x1802,
};

enum class PixelFormat : core::EnumType
{
  Alpha = 0x1906,
  Blue = 0x1905,
  DepthComponent = 0x1902,
  Green = 0x1904,
  Red = 0x1903,
  Rgb = 0x1907,
  Rgba = 0x1908,
  StencilIndex = 0x1901,
  UnsignedInt = 0x1405,
  UnsignedShort = 0x1403,
  Bgr = 0x80E0,
  Bgra = 0x80E1,
  BgraInteger = 0x8D9B,
  BgrInteger = 0x8D9A,
  BlueInteger = 0x8D96,
  DepthStencil = 0x84F9,
  GreenInteger = 0x8D95,
  RedInteger = 0x8D94,
  Rg = 0x8227,
  RgbaInteger = 0x8D99,
  RgbInteger = 0x8D98,
  RgInteger = 0x8228,
};

enum class PixelStoreParameter : core::EnumType
{
  PackAlignment = 0x0D05,
  PackLsbFirst = 0x0D01,
  PackRowLength = 0x0D02,
  PackSkipPixels = 0x0D04,
  PackSkipRows = 0x0D03,
  PackSwapBytes = 0x0D00,
  UnpackAlignment = 0x0CF5,
  UnpackLsbFirst = 0x0CF1,
  UnpackRowLength = 0x0CF2,
  UnpackSkipPixels = 0x0CF4,
  UnpackSkipRows = 0x0CF3,
  UnpackSwapBytes = 0x0CF0,
  PackImageHeight = 0x806C,
  PackSkipImages = 0x806B,
  UnpackImageHeight = 0x806E,
  UnpackSkipImages = 0x806D,
};

enum class PixelTexGenModeSGIX : core::EnumType
{
  Alpha = 0x1906,
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
  UnsignedByte233Rev = 0x8362,
  UnsignedByte332 = 0x8032,
  UnsignedInt1010102 = 0x8036,
  UnsignedInt2101010Rev = 0x8368,
  UnsignedInt8888 = 0x8035,
  UnsignedInt8888Rev = 0x8367,
  UnsignedShort1555Rev = 0x8366,
  UnsignedShort4444 = 0x8033,
  UnsignedShort4444Rev = 0x8365,
  UnsignedShort5551 = 0x8034,
  UnsignedShort565 = 0x8363,
  UnsignedShort565Rev = 0x8364,
  Float32UnsignedInt248Rev = 0x8DAD,
  HalfFloat = 0x140B,
  UnsignedInt10f11f11fRev = 0x8C3B,
  UnsignedInt248 = 0x84FA,
  UnsignedInt5999Rev = 0x8C3E,
};

enum class PointParameterName : core::EnumType
{
  PointFadeThresholdSize = 0x8128,
};

enum class PolygonMode : core::EnumType
{
  Fill = 0x1B02,
  Line = 0x1B01,
  Point = 0x1B00,
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
  Quads = 0x0007,
  LinesAdjacency = 0x000A,
  LineStripAdjacency = 0x000B,
  TrianglesAdjacency = 0x000C,
  TriangleStripAdjacency = 0x000D,
  Patches = 0x000E,
};

enum class ProgramInterface : core::EnumType
{
  TransformFeedbackBuffer = 0x8C8E,
  BufferVariable = 0x92E5,
  ComputeSubroutine = 0x92ED,
  ComputeSubroutineUniform = 0x92F3,
  FragmentSubroutine = 0x92EC,
  FragmentSubroutineUniform = 0x92F2,
  GeometrySubroutine = 0x92EB,
  GeometrySubroutineUniform = 0x92F1,
  ProgramInput = 0x92E3,
  ProgramOutput = 0x92E4,
  ShaderStorageBlock = 0x92E6,
  TessControlSubroutine = 0x92E9,
  TessControlSubroutineUniform = 0x92EF,
  TessEvaluationSubroutine = 0x92EA,
  TessEvaluationSubroutineUniform = 0x92F0,
  TransformFeedbackVarying = 0x92F4,
  Uniform = 0x92E1,
  UniformBlock = 0x92E2,
  VertexSubroutine = 0x92E8,
  VertexSubroutineUniform = 0x92EE,
};

enum class ProgramInterfacePName : core::EnumType
{
  ActiveResources = 0x92F5,
  MaxNameLength = 0x92F6,
  MaxNumActiveVariables = 0x92F7,
  MaxNumCompatibleSubroutines = 0x92F8,
};

enum class ProgramParameterPName : core::EnumType
{
  ProgramBinaryRetrievableHint = 0x8257,
  ProgramSeparable = 0x8258,
};

enum class ProgramProperty : core::EnumType
{
  ActiveAttributes = 0x8B89,
  ActiveAttributeMaxLength = 0x8B8A,
  ActiveUniforms = 0x8B86,
  ActiveUniformMaxLength = 0x8B87,
  AttachedShaders = 0x8B85,
  DeleteStatus = 0x8B80,
  InfoLogLength = 0x8B84,
  LinkStatus = 0x8B82,
  ValidateStatus = 0x8B83,
  TransformFeedbackBufferMode = 0x8C7F,
  TransformFeedbackVaryings = 0x8C83,
  TransformFeedbackVaryingMaxLength = 0x8C76,
  ActiveUniformBlocks = 0x8A36,
  ActiveUniformBlockMaxNameLength = 0x8A35,
  GeometryInputType = 0x8917,
  GeometryOutputType = 0x8918,
  GeometryVerticesOut = 0x8916,
  ProgramBinaryLength = 0x8741,
  ActiveAtomicCounterBuffers = 0x92D9,
  ComputeWorkGroupSize = 0x8267,
};

enum class ProgramResourceProperty : core::EnumType
{
  CompatibleSubroutines = 0x8E4B,
  NumCompatibleSubroutines = 0x8E4A,
  ActiveVariables = 0x9305,
  ArraySize = 0x92FB,
  ArrayStride = 0x92FE,
  AtomicCounterBufferIndex = 0x9301,
  BlockIndex = 0x92FD,
  BufferBinding = 0x9302,
  BufferDataSize = 0x9303,
  IsPerPatch = 0x92E7,
  IsRowMajor = 0x9300,
  Location = 0x930E,
  LocationIndex = 0x930F,
  MatrixStride = 0x92FF,
  NameLength = 0x92F9,
  NumActiveVariables = 0x9304,
  Offset = 0x92FC,
  ReferencedByComputeShader = 0x930B,
  ReferencedByFragmentShader = 0x930A,
  ReferencedByGeometryShader = 0x9309,
  ReferencedByTessControlShader = 0x9307,
  ReferencedByTessEvaluationShader = 0x9308,
  ReferencedByVertexShader = 0x9306,
  TopLevelArraySize = 0x930C,
  TopLevelArrayStride = 0x930D,
  Type = 0x92FA,
  Uniform = 0x92E1,
  LocationComponent = 0x934A,
  TransformFeedbackBufferIndex = 0x934B,
  TransformFeedbackBufferStride = 0x934C,
};

enum class ProgramStagePName : core::EnumType
{
  ActiveSubroutines = 0x8DE5,
  ActiveSubroutineMaxLength = 0x8E48,
  ActiveSubroutineUniforms = 0x8DE6,
  ActiveSubroutineUniformLocations = 0x8E47,
  ActiveSubroutineUniformMaxLength = 0x8E49,
};

enum class QueryCounterTarget : core::EnumType
{
  Timestamp = 0x8E28,
};

enum class QueryObjectParameterName : core::EnumType
{
  QueryResult = 0x8866,
  QueryResultAvailable = 0x8867,
  QueryResultNoWait = 0x9194,
  QueryTarget = 0x82EA,
};

enum class QueryParameterName : core::EnumType
{
  CurrentQuery = 0x8865,
  QueryCounterBits = 0x8864,
};

enum class QueryTarget : core::EnumType
{
  SamplesPassed = 0x8914,
  PrimitivesGenerated = 0x8C87,
  TransformFeedbackPrimitivesWritten = 0x8C88,
  AnySamplesPassed = 0x8C2F,
  TimeElapsed = 0x88BF,
  AnySamplesPassedConservative = 0x8D6A,
};

enum class ReadBufferMode : core::EnumType
{
  Back = 0x0405,
  BackLeft = 0x0402,
  BackRight = 0x0403,
  Front = 0x0404,
  FrontLeft = 0x0400,
  FrontRight = 0x0401,
  Left = 0x0406,
  None = 0,
  Right = 0x0407,
  ColorAttachment0 = 0x8CE0,
  ColorAttachment1 = 0x8CE1,
  ColorAttachment10 = 0x8CEA,
  ColorAttachment11 = 0x8CEB,
  ColorAttachment12 = 0x8CEC,
  ColorAttachment13 = 0x8CED,
  ColorAttachment14 = 0x8CEE,
  ColorAttachment15 = 0x8CEF,
  ColorAttachment2 = 0x8CE2,
  ColorAttachment3 = 0x8CE3,
  ColorAttachment4 = 0x8CE4,
  ColorAttachment5 = 0x8CE5,
  ColorAttachment6 = 0x8CE6,
  ColorAttachment7 = 0x8CE7,
  ColorAttachment8 = 0x8CE8,
  ColorAttachment9 = 0x8CE9,
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
  RenderbufferSamples = 0x8CAB,
  RenderbufferStencilSize = 0x8D55,
  RenderbufferWidth = 0x8D42,
};

enum class RenderbufferTarget : core::EnumType
{
  Renderbuffer = 0x8D41,
};

enum class ReplacementCodeTypeSUN : core::EnumType
{
  UnsignedByte = 0x1401,
  UnsignedInt = 0x1405,
  UnsignedShort = 0x1403,
};

enum class SamplerParameterF : core::EnumType
{
  TextureBorderColor = 0x1004,
  TextureMaxAnisotropy = 0x84FE,
  TextureMaxLod = 0x813B,
  TextureMinLod = 0x813A,
  TextureLodBias = 0x8501,
};

enum class SamplerParameterI : core::EnumType
{
  TextureMagFilter = 0x2800,
  TextureMinFilter = 0x2801,
  TextureWrapS = 0x2802,
  TextureWrapT = 0x2803,
  TextureWrapR = 0x8072,
  TextureCompareFunc = 0x884D,
  TextureCompareMode = 0x884C,
};

enum class ScalarType : core::EnumType
{
  UnsignedByte = 0x1401,
  UnsignedInt = 0x1405,
  UnsignedShort = 0x1403,
};

enum class SecondaryColorPointerTypeIBM : core::EnumType
{
  Float = 0x1406,
  Int = 0x1404,
  Short = 0x1402,
  Double = 0x140A,
};

enum class ShaderParameterName : core::EnumType
{
  CompileStatus = 0x8B81,
  DeleteStatus = 0x8B80,
  InfoLogLength = 0x8B84,
  ShaderSourceLength = 0x8B88,
  ShaderType = 0x8B4F,
};

enum class ShaderType : core::EnumType
{
  FragmentShader = 0x8B30,
  VertexShader = 0x8B31,
  GeometryShader = 0x8DD9,
  TessControlShader = 0x8E88,
  TessEvaluationShader = 0x8E87,
  ComputeShader = 0x91B9,
};

enum class SizedInternalFormat : core::EnumType
{
  R3G3B2 = 0x2A10,
  Rgb10 = 0x8052,
  Rgb10A2 = 0x8059,
  Rgb12 = 0x8053,
  Rgb16 = 0x8054,
  Rgb4 = 0x804F,
  Rgb5 = 0x8050,
  Rgb5A1 = 0x8057,
  Rgb8 = 0x8051,
  Rgba12 = 0x805A,
  Rgba16 = 0x805B,
  Rgba2 = 0x8055,
  Rgba4 = 0x8056,
  Rgba8 = 0x8058,
  DepthComponent16 = 0x81A5,
  DepthComponent24 = 0x81A6,
  DepthComponent32 = 0x81A7,
  Srgb8 = 0x8C41,
  Srgb8Alpha8 = 0x8C43,
  CompressedRedRgtc1 = 0x8DBB,
  CompressedRgRgtc2 = 0x8DBD,
  CompressedSignedRedRgtc1 = 0x8DBC,
  CompressedSignedRgRgtc2 = 0x8DBE,
  Depth24Stencil8 = 0x88F0,
  Depth32fStencil8 = 0x8CAD,
  DepthComponent32f = 0x8CAC,
  R11fG11fB10f = 0x8C3A,
  R16 = 0x822A,
  R16f = 0x822D,
  R16i = 0x8233,
  R16ui = 0x8234,
  R32f = 0x822E,
  R32i = 0x8235,
  R32ui = 0x8236,
  R8 = 0x8229,
  R8i = 0x8231,
  R8ui = 0x8232,
  Rg16 = 0x822C,
  Rg16f = 0x822F,
  Rg16i = 0x8239,
  Rg16ui = 0x823A,
  Rg32f = 0x8230,
  Rg32i = 0x823B,
  Rg32ui = 0x823C,
  Rg8 = 0x822B,
  Rg8i = 0x8237,
  Rg8ui = 0x8238,
  Rgb16f = 0x881B,
  Rgb16i = 0x8D89,
  Rgb16ui = 0x8D77,
  Rgb32f = 0x8815,
  Rgb32i = 0x8D83,
  Rgb32ui = 0x8D71,
  Rgb8i = 0x8D8F,
  Rgb8ui = 0x8D7D,
  Rgb9E5 = 0x8C3D,
  Rgba16f = 0x881A,
  Rgba16i = 0x8D88,
  Rgba16ui = 0x8D76,
  Rgba32f = 0x8814,
  Rgba32i = 0x8D82,
  Rgba32ui = 0x8D70,
  Rgba8i = 0x8D8E,
  Rgba8ui = 0x8D7C,
  StencilIndex1 = 0x8D46,
  StencilIndex16 = 0x8D49,
  StencilIndex4 = 0x8D47,
  StencilIndex8 = 0x8D48,
  R16Snorm = 0x8F98,
  R8Snorm = 0x8F94,
  Rg16Snorm = 0x8F99,
  Rg8Snorm = 0x8F95,
  Rgb16Snorm = 0x8F9A,
  Rgb8Snorm = 0x8F96,
  Rgba16Snorm = 0x8F9B,
  Rgba8Snorm = 0x8F97,
  Rgb10A2ui = 0x906F,
  Rgb565 = 0x8D62,
  CompressedRgbaBptcUnorm = 0x8E8C,
  CompressedRgbBptcSignedFloat = 0x8E8E,
  CompressedRgbBptcUnsignedFloat = 0x8E8F,
  CompressedSrgbAlphaBptcUnorm = 0x8E8D,
  CompressedR11Eac = 0x9270,
  CompressedRg11Eac = 0x9272,
  CompressedRgb8Etc2 = 0x9274,
  CompressedRgb8PunchthroughAlpha1Etc2 = 0x9276,
  CompressedRgba8Etc2Eac = 0x9278,
  CompressedSignedR11Eac = 0x9271,
  CompressedSignedRg11Eac = 0x9273,
  CompressedSrgb8Alpha8Etc2Eac = 0x9279,
  CompressedSrgb8Etc2 = 0x9275,
  CompressedSrgb8PunchthroughAlpha1Etc2 = 0x9277,
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
  Incr = 0x1E02,
  Invert = 0x150A,
  Keep = 0x1E00,
  Replace = 0x1E01,
  Zero = 0,
  DecrWrap = 0x8508,
  IncrWrap = 0x8507,
};

enum class StringName : core::EnumType
{
  Extensions = 0x1F03,
  Renderer = 0x1F01,
  Vendor = 0x1F00,
  Version = 0x1F02,
  ShadingLanguageVersion = 0x8B8C,
};

enum class SubroutineParameterName : core::EnumType
{
  UniformNameLength = 0x8A39,
  UniformSize = 0x8A38,
  CompatibleSubroutines = 0x8E4B,
  NumCompatibleSubroutines = 0x8E4A,
};

enum class SyncBehaviorFlags : core::EnumType
{
  None = 0,
};

enum class SyncCondition : core::EnumType
{
  SyncGpuCommandsComplete = 0x9117,
};

enum class SyncObjectMask : core::EnumType
{
  SyncFlushCommandsBit = 0x00000001,
};
constexpr core::Bitfield<SyncObjectMask> operator|(SyncObjectMask left, SyncObjectMask right)
{
  return core::Bitfield<SyncObjectMask>(left) | right;
}

enum class SyncParameterName : core::EnumType
{
  ObjectType = 0x9112,
  SyncCondition = 0x9113,
  SyncFlags = 0x9115,
  SyncStatus = 0x9114,
};

enum class SyncStatus : core::EnumType
{
  AlreadySignaled = 0x911A,
  ConditionSatisfied = 0x911C,
  TimeoutExpired = 0x911B,
  WaitFailed = 0x911D,
};

enum class TangentPointerTypeEXT : core::EnumType
{
  Byte = 0x1400,
  Float = 0x1406,
  Int = 0x1404,
  Short = 0x1402,
  Double = 0x140A,
};

enum class TexCoordPointerType : core::EnumType
{
  Float = 0x1406,
  Int = 0x1404,
  Short = 0x1402,
  Double = 0x140A,
};

enum class TextureCompareMode : core::EnumType
{
  None = 0,
  CompareRefToTexture = 0x884E,
};

enum class TextureEnvMode : core::EnumType
{
  Blend = 0x0BE2,
  Replace = 0x1E01,
};

enum class TextureEnvParameter : core::EnumType
{
  TextureLodBias = 0x8501,
  Src1Alpha = 0x8589,
};

enum class TextureEnvTarget : core::EnumType
{
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
  TextureBorderColor = 0x1004,
  TextureHeight = 0x1001,
  TextureMagFilter = 0x2800,
  TextureMaxAnisotropy = 0x84FE,
  TextureMinFilter = 0x2801,
  TextureWidth = 0x1000,
  TextureWrapS = 0x2802,
  TextureWrapT = 0x2803,
  TextureAlphaSize = 0x805F,
  TextureBlueSize = 0x805E,
  TextureGreenSize = 0x805D,
  TextureInternalFormat = 0x1003,
  TextureRedSize = 0x805C,
  TextureBaseLevel = 0x813C,
  TextureMaxLevel = 0x813D,
  TextureMaxLod = 0x813B,
  TextureMinLod = 0x813A,
  TextureWrapR = 0x8072,
  TextureCompareFunc = 0x884D,
  TextureCompareMode = 0x884C,
  TextureLodBias = 0x8501,
  TextureSwizzleA = 0x8E45,
  TextureSwizzleB = 0x8E44,
  TextureSwizzleG = 0x8E43,
  TextureSwizzleR = 0x8E42,
  TextureSwizzleRgba = 0x8E46,
  DepthStencilTextureMode = 0x90EA,
};

enum class TextureSwizzle : core::EnumType
{
  Alpha = 0x1906,
  Blue = 0x1905,
  Green = 0x1904,
  One = 1,
  Red = 0x1903,
  Zero = 0,
};

enum class TextureTarget : core::EnumType
{
  Texture1d = 0x0DE0,
  Texture2d = 0x0DE1,
  ProxyTexture1d = 0x8063,
  ProxyTexture2d = 0x8064,
  ProxyTexture3d = 0x8070,
  Texture3d = 0x806F,
  ProxyTextureCubeMap = 0x851B,
  TextureCubeMap = 0x8513,
  TextureCubeMapNegativeX = 0x8516,
  TextureCubeMapNegativeY = 0x8518,
  TextureCubeMapNegativeZ = 0x851A,
  TextureCubeMapPositiveX = 0x8515,
  TextureCubeMapPositiveY = 0x8517,
  TextureCubeMapPositiveZ = 0x8519,
  ProxyTexture1dArray = 0x8C19,
  ProxyTexture2dArray = 0x8C1B,
  Renderbuffer = 0x8D41,
  Texture1dArray = 0x8C18,
  Texture2dArray = 0x8C1A,
  ProxyTextureRectangle = 0x84F7,
  TextureBuffer = 0x8C2A,
  TextureRectangle = 0x84F5,
  ProxyTexture2dMultisample = 0x9101,
  ProxyTexture2dMultisampleArray = 0x9103,
  Texture2dMultisample = 0x9100,
  Texture2dMultisampleArray = 0x9102,
  ProxyTextureCubeMapArray = 0x900B,
  TextureCubeMapArray = 0x9009,
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
  Repeat = 0x2901,
  ClampToEdge = 0x812F,
  ClampToBorder = 0x812D,
  MirroredRepeat = 0x8370,
};

enum class TransformFeedbackBufferMode : core::EnumType
{
  InterleavedAttribs = 0x8C8C,
  SeparateAttribs = 0x8C8D,
};

enum class TransformFeedbackPName : core::EnumType
{
  TransformFeedbackBufferBinding = 0x8C8F,
  TransformFeedbackBufferSize = 0x8C85,
  TransformFeedbackBufferStart = 0x8C84,
  TransformFeedbackActive = 0x8E24,
  TransformFeedbackPaused = 0x8E23,
};

enum class TriangleFace : core::EnumType
{
  Back = 0x0405,
  Front = 0x0404,
  FrontAndBack = 0x0408,
};

enum class UniformBlockPName : core::EnumType
{
  UniformBlockActiveUniforms = 0x8A42,
  UniformBlockActiveUniformIndices = 0x8A43,
  UniformBlockBinding = 0x8A3F,
  UniformBlockDataSize = 0x8A40,
  UniformBlockNameLength = 0x8A41,
  UniformBlockReferencedByFragmentShader = 0x8A46,
  UniformBlockReferencedByGeometryShader = 0x8A45,
  UniformBlockReferencedByVertexShader = 0x8A44,
  UniformBlockReferencedByTessControlShader = 0x84F0,
  UniformBlockReferencedByTessEvaluationShader = 0x84F1,
  UniformBlockReferencedByComputeShader = 0x90EC,
};

enum class UniformPName : core::EnumType
{
  UniformArrayStride = 0x8A3C,
  UniformBlockIndex = 0x8A3A,
  UniformIsRowMajor = 0x8A3E,
  UniformMatrixStride = 0x8A3D,
  UniformNameLength = 0x8A39,
  UniformOffset = 0x8A3B,
  UniformSize = 0x8A38,
  UniformType = 0x8A37,
  UniformAtomicCounterBufferIndex = 0x92DA,
};

enum class UniformType : core::EnumType
{
  Float = 0x1406,
  Int = 0x1404,
  UnsignedInt = 0x1405,
  Double = 0x140A,
  Bool = 0x8B56,
  BoolVec2 = 0x8B57,
  BoolVec3 = 0x8B58,
  BoolVec4 = 0x8B59,
  FloatMat2 = 0x8B5A,
  FloatMat3 = 0x8B5B,
  FloatMat4 = 0x8B5C,
  FloatVec2 = 0x8B50,
  FloatVec3 = 0x8B51,
  FloatVec4 = 0x8B52,
  IntVec2 = 0x8B53,
  IntVec3 = 0x8B54,
  IntVec4 = 0x8B55,
  Sampler1d = 0x8B5D,
  Sampler1dShadow = 0x8B61,
  Sampler2d = 0x8B5E,
  Sampler2dShadow = 0x8B62,
  Sampler3d = 0x8B5F,
  SamplerCube = 0x8B60,
  FloatMat2x3 = 0x8B65,
  FloatMat2x4 = 0x8B66,
  FloatMat3x2 = 0x8B67,
  FloatMat3x4 = 0x8B68,
  FloatMat4x2 = 0x8B69,
  FloatMat4x3 = 0x8B6A,
  IntSampler1d = 0x8DC9,
  IntSampler1dArray = 0x8DCE,
  IntSampler2d = 0x8DCA,
  IntSampler2dArray = 0x8DCF,
  IntSampler3d = 0x8DCB,
  IntSamplerCube = 0x8DCC,
  Sampler1dArray = 0x8DC0,
  Sampler1dArrayShadow = 0x8DC3,
  Sampler2dArray = 0x8DC1,
  Sampler2dArrayShadow = 0x8DC4,
  SamplerCubeShadow = 0x8DC5,
  UnsignedIntSampler1d = 0x8DD1,
  UnsignedIntSampler1dArray = 0x8DD6,
  UnsignedIntSampler2d = 0x8DD2,
  UnsignedIntSampler2dArray = 0x8DD7,
  UnsignedIntSampler3d = 0x8DD3,
  UnsignedIntSamplerCube = 0x8DD4,
  UnsignedIntVec2 = 0x8DC6,
  UnsignedIntVec3 = 0x8DC7,
  UnsignedIntVec4 = 0x8DC8,
  IntSampler2dRect = 0x8DCD,
  IntSamplerBuffer = 0x8DD0,
  Sampler2dRect = 0x8B63,
  Sampler2dRectShadow = 0x8B64,
  SamplerBuffer = 0x8DC2,
  UnsignedIntSampler2dRect = 0x8DD5,
  UnsignedIntSamplerBuffer = 0x8DD8,
  IntSampler2dMultisample = 0x9109,
  IntSampler2dMultisampleArray = 0x910C,
  Sampler2dMultisample = 0x9108,
  Sampler2dMultisampleArray = 0x910B,
  UnsignedIntSampler2dMultisample = 0x910A,
  UnsignedIntSampler2dMultisampleArray = 0x910D,
  DoubleMat2 = 0x8F46,
  DoubleMat2x3 = 0x8F49,
  DoubleMat2x4 = 0x8F4A,
  DoubleMat3 = 0x8F47,
  DoubleMat3x2 = 0x8F4B,
  DoubleMat3x4 = 0x8F4C,
  DoubleMat4 = 0x8F48,
  DoubleMat4x2 = 0x8F4D,
  DoubleMat4x3 = 0x8F4E,
  DoubleVec2 = 0x8FFC,
  DoubleVec3 = 0x8FFD,
  DoubleVec4 = 0x8FFE,
  IntSamplerCubeMapArray = 0x900E,
  SamplerCubeMapArray = 0x900C,
  SamplerCubeMapArrayShadow = 0x900D,
  UnsignedIntSamplerCubeMapArray = 0x900F,
};

enum class UseProgramStageMask : core::EnumType
{
  AllShaderBits = 0xFFFFFFFF,
  FragmentShaderBit = 0x00000002,
  GeometryShaderBit = 0x00000004,
  TessControlShaderBit = 0x00000008,
  TessEvaluationShaderBit = 0x00000010,
  VertexShaderBit = 0x00000001,
  ComputeShaderBit = 0x00000020,
};
constexpr core::Bitfield<UseProgramStageMask> operator|(UseProgramStageMask left, UseProgramStageMask right)
{
  return core::Bitfield<UseProgramStageMask>(left) | right;
}

enum class VertexArrayPName : core::EnumType
{
  VertexAttribArrayEnabled = 0x8622,
  VertexAttribArrayNormalized = 0x886A,
  VertexAttribArraySize = 0x8623,
  VertexAttribArrayStride = 0x8624,
  VertexAttribArrayType = 0x8625,
  VertexAttribArrayInteger = 0x88FD,
  VertexAttribArrayDivisor = 0x88FE,
  VertexAttribArrayLong = 0x874E,
  VertexAttribRelativeOffset = 0x82D5,
};

enum class VertexAttribEnum : core::EnumType
{
  VertexAttribArrayBufferBinding = 0x889F,
  CurrentVertexAttrib = 0x8626,
  VertexAttribArrayEnabled = 0x8622,
  VertexAttribArrayNormalized = 0x886A,
  VertexAttribArraySize = 0x8623,
  VertexAttribArrayStride = 0x8624,
  VertexAttribArrayType = 0x8625,
  VertexAttribArrayInteger = 0x88FD,
  VertexAttribArrayDivisor = 0x88FE,
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

enum class VertexAttribLType : core::EnumType
{
  Double = 0x140A,
};

enum class VertexAttribPointerProperty : core::EnumType
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
  Double = 0x140A,
  UnsignedInt2101010Rev = 0x8368,
  HalfFloat = 0x140B,
  UnsignedInt10f11f11fRev = 0x8C3B,
  Int2101010Rev = 0x8D9F,
  Fixed = 0x140C,
};

enum class VertexAttribProperty : core::EnumType
{
  VertexAttribArrayBufferBinding = 0x889F,
  CurrentVertexAttrib = 0x8626,
  VertexAttribArrayEnabled = 0x8622,
  VertexAttribArrayNormalized = 0x886A,
  VertexAttribArraySize = 0x8623,
  VertexAttribArrayStride = 0x8624,
  VertexAttribArrayType = 0x8625,
  VertexAttribArrayInteger = 0x88FD,
  VertexAttribArrayDivisor = 0x88FE,
  VertexAttribArrayLong = 0x874E,
  VertexAttribBinding = 0x82D4,
  VertexAttribRelativeOffset = 0x82D5,
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
  Double = 0x140A,
  UnsignedInt2101010Rev = 0x8368,
  HalfFloat = 0x140B,
  UnsignedInt10f11f11fRev = 0x8C3B,
  Int2101010Rev = 0x8D9F,
  Fixed = 0x140C,
};

enum class VertexBufferObjectUsage : core::EnumType
{
  DynamicCopy = 0x88EA,
  DynamicDraw = 0x88E8,
  DynamicRead = 0x88E9,
  StaticCopy = 0x88E6,
  StaticDraw = 0x88E4,
  StaticRead = 0x88E5,
  StreamCopy = 0x88E2,
  StreamDraw = 0x88E0,
  StreamRead = 0x88E1,
};

enum class VertexPointerType : core::EnumType
{
  Float = 0x1406,
  Int = 0x1404,
  Short = 0x1402,
  Double = 0x140A,
};

enum class VertexProvokingMode : core::EnumType
{
  FirstVertexConvention = 0x8E4D,
  LastVertexConvention = 0x8E4E,
};

enum class VertexShaderWriteMaskEXT : core::EnumType
{
  False = 0,
  True = 1,
};

enum class VertexWeightPointerTypeEXT : core::EnumType
{
  Float = 0x1406,
};

enum class WeightPointerType : core::EnumType
{
  Byte = 0x1400,
  Float = 0x1406,
  Int = 0x1404,
  Short = 0x1402,
  UnsignedByte = 0x1401,
  UnsignedInt = 0x1405,
  UnsignedShort = 0x1403,
  Double = 0x140A,
};

// commands
extern void blendFunc(BlendingFactor sfactor, BlendingFactor dfactor);
extern void clear(core::Bitfield<ClearBufferMask> mask);
extern void clearColor(float red, float green, float blue, float alpha);
extern void clearDepth(double depth);
extern void clearStencil(int32_t s);
extern SyncStatus clientWaitSync(core::Sync sync, core::Bitfield<SyncObjectMask> flags, uint64_t timeout);
extern void colorMask(bool red, bool green, bool blue, bool alpha);
extern void cullFace(TriangleFace mode);
extern void deleteSync(core::Sync sync);
extern void depthFunc(DepthFunction func);
extern void depthMask(bool flag);
extern void depthRange(double n, double f);
extern void disable(EnableCap cap);
extern void drawBuffer(DrawBufferMode buf);
extern void enable(EnableCap cap);
extern core::Sync fenceSync(SyncCondition condition, core::Bitfield<SyncBehaviorFlags> flags);
extern void finish();
extern void flush();
extern void frontFace(FrontFaceDirection mode);
extern void getBoolean(GetPName pname, bool* data);
extern void getDouble(GetPName pname, double* data);
extern ErrorCode getError();
extern void getFloat(GetPName pname, float* data);
extern uint64_t getImageHandle(uint32_t texture, int32_t level, bool layered, int32_t layer, PixelFormat format);
extern void getInteger64(GetPName pname, int64_t* data);
extern void getInteger(GetPName pname, int32_t* data);
extern const uint8_t* getString(StringName name);
extern void
  getSync(core::Sync sync, SyncParameterName pname, core::SizeType count, core::SizeType* length, int32_t* values);
extern void getTexImage(TextureTarget target, int32_t level, PixelFormat format, PixelType type, void* pixels);
extern void getTexLevelParameter(TextureTarget target, int32_t level, GetTextureParameter pname, float* params);
extern void getTexLevelParameter(TextureTarget target, int32_t level, GetTextureParameter pname, int32_t* params);
extern void getTexParameter(TextureTarget target, GetTextureParameter pname, float* params);
extern void getTexParameter(TextureTarget target, GetTextureParameter pname, int32_t* params);
extern uint64_t getTextureHandle(uint32_t texture);
extern uint64_t getTextureSamplerHandle(uint32_t texture, uint32_t sampler);
extern void getVertexAttribL(uint32_t index, VertexAttribEnum pname, uint64_t* params);
extern void hint(HintTarget target, HintMode mode);
extern bool isEnabled(EnableCap cap);
extern bool isImageHandleResident(uint64_t handle);
extern bool isSync(core::Sync sync);
extern bool isTextureHandleResident(uint64_t handle);
extern void lineWidth(float width);
extern void logicOp(LogicOp opcode);
extern void makeImageHandleNonResident(uint64_t handle);
extern void makeImageHandleResident(uint64_t handle, core::EnumType access);
extern void makeTextureHandleNonResident(uint64_t handle);
extern void makeTextureHandleResident(uint64_t handle);
extern void pixelStore(PixelStoreParameter pname, float param);
extern void pixelStore(PixelStoreParameter pname, int32_t param);
extern void pointSize(float size);
extern void polygonMode(TriangleFace face, PolygonMode mode);
extern void programUniformHandle(uint32_t program, int32_t location, uint64_t value);
extern void programUniformHandle(uint32_t program, int32_t location, core::SizeType count, const uint64_t* values);
extern void readBuffer(ReadBufferMode src);
extern void readPixel(
  int32_t x, int32_t y, core::SizeType width, core::SizeType height, PixelFormat format, PixelType type, void* pixels);
extern void scissor(int32_t x, int32_t y, core::SizeType width, core::SizeType height);
extern void stencilFunc(StencilFunction func, int32_t ref, uint32_t mask);
extern void stencilMask(uint32_t mask);
extern void stencilOp(StencilOp fail, StencilOp zfail, StencilOp zpass);
extern void texImage1D(TextureTarget target,
                       int32_t level,
                       int32_t internalformat,
                       core::SizeType width,
                       int32_t border,
                       PixelFormat format,
                       PixelType type,
                       const void* pixels);
extern void texImage2D(TextureTarget target,
                       int32_t level,
                       InternalFormat internalformat,
                       core::SizeType width,
                       core::SizeType height,
                       int32_t border,
                       PixelFormat format,
                       PixelType type,
                       const void* pixels);
extern void texParameter(TextureTarget target, TextureParameterName pname, float param);
extern void texParameter(TextureTarget target, TextureParameterName pname, const float* params);
extern void texParameter(TextureTarget target, TextureParameterName pname, int32_t param);
extern void texParameter(TextureTarget target, TextureParameterName pname, const int32_t* params);
extern void uniformHandle(int32_t location, uint64_t value);
extern void uniformHandle(int32_t location, core::SizeType count, const uint64_t* value);
extern void vertexAttribL1(uint32_t index, uint64_t x);
extern void vertexAttribL1(uint32_t index, const uint64_t* v);
extern void viewport(int32_t x, int32_t y, core::SizeType width, core::SizeType height);
extern void waitSync(core::Sync sync, core::Bitfield<SyncBehaviorFlags> flags, uint64_t timeout);
extern void bindTexture(TextureTarget target, uint32_t texture);
extern void copyTexImage1D(TextureTarget target,
                           int32_t level,
                           InternalFormat internalformat,
                           int32_t x,
                           int32_t y,
                           core::SizeType width,
                           int32_t border);
extern void copyTexImage2D(TextureTarget target,
                           int32_t level,
                           InternalFormat internalformat,
                           int32_t x,
                           int32_t y,
                           core::SizeType width,
                           core::SizeType height,
                           int32_t border);
extern void
  copyTexSubImage1D(TextureTarget target, int32_t level, int32_t xoffset, int32_t x, int32_t y, core::SizeType width);
extern void copyTexSubImage2D(TextureTarget target,
                              int32_t level,
                              int32_t xoffset,
                              int32_t yoffset,
                              int32_t x,
                              int32_t y,
                              core::SizeType width,
                              core::SizeType height);
extern void deleteTextures(core::SizeType n, const uint32_t* textures);
extern void drawArrays(PrimitiveType mode, int32_t first, core::SizeType count);
extern void drawElements(PrimitiveType mode, core::SizeType count, DrawElementsType type, const void* indices);
extern void genTextures(core::SizeType n, uint32_t* textures);
extern bool isTexture(uint32_t texture);
extern void polygonOffset(float factor, float units);
extern void texSubImage1D(TextureTarget target,
                          int32_t level,
                          int32_t xoffset,
                          core::SizeType width,
                          PixelFormat format,
                          PixelType type,
                          const void* pixels);
extern void texSubImage2D(TextureTarget target,
                          int32_t level,
                          int32_t xoffset,
                          int32_t yoffset,
                          core::SizeType width,
                          core::SizeType height,
                          PixelFormat format,
                          PixelType type,
                          const void* pixels);
extern void getPointer(GetPointervPName pname, void** params);
extern void copyTexSubImage3D(TextureTarget target,
                              int32_t level,
                              int32_t xoffset,
                              int32_t yoffset,
                              int32_t zoffset,
                              int32_t x,
                              int32_t y,
                              core::SizeType width,
                              core::SizeType height);
extern void drawRangeElements(
  PrimitiveType mode, uint32_t start, uint32_t end, core::SizeType count, DrawElementsType type, const void* indices);
extern void texImage3D(TextureTarget target,
                       int32_t level,
                       InternalFormat internalformat,
                       core::SizeType width,
                       core::SizeType height,
                       core::SizeType depth,
                       int32_t border,
                       PixelFormat format,
                       PixelType type,
                       const void* pixels);
extern void texSubImage3D(TextureTarget target,
                          int32_t level,
                          int32_t xoffset,
                          int32_t yoffset,
                          int32_t zoffset,
                          core::SizeType width,
                          core::SizeType height,
                          core::SizeType depth,
                          PixelFormat format,
                          PixelType type,
                          const void* pixels);
extern void activeTexture(TextureUnit texture);
extern void compressedTexImage1D(TextureTarget target,
                                 int32_t level,
                                 InternalFormat internalformat,
                                 core::SizeType width,
                                 int32_t border,
                                 core::SizeType imageSize,
                                 const void* data);
extern void compressedTexImage2D(TextureTarget target,
                                 int32_t level,
                                 InternalFormat internalformat,
                                 core::SizeType width,
                                 core::SizeType height,
                                 int32_t border,
                                 core::SizeType imageSize,
                                 const void* data);
extern void compressedTexImage3D(TextureTarget target,
                                 int32_t level,
                                 InternalFormat internalformat,
                                 core::SizeType width,
                                 core::SizeType height,
                                 core::SizeType depth,
                                 int32_t border,
                                 core::SizeType imageSize,
                                 const void* data);
extern void compressedTexSubImage1D(TextureTarget target,
                                    int32_t level,
                                    int32_t xoffset,
                                    core::SizeType width,
                                    InternalFormat format,
                                    core::SizeType imageSize,
                                    const void* data);
extern void compressedTexSubImage2D(TextureTarget target,
                                    int32_t level,
                                    int32_t xoffset,
                                    int32_t yoffset,
                                    core::SizeType width,
                                    core::SizeType height,
                                    InternalFormat format,
                                    core::SizeType imageSize,
                                    const void* data);
extern void compressedTexSubImage3D(TextureTarget target,
                                    int32_t level,
                                    int32_t xoffset,
                                    int32_t yoffset,
                                    int32_t zoffset,
                                    core::SizeType width,
                                    core::SizeType height,
                                    core::SizeType depth,
                                    InternalFormat format,
                                    core::SizeType imageSize,
                                    const void* data);
extern void getCompressedTexImage(TextureTarget target, int32_t level, void* img);
extern void sampleCoverage(float value, bool invert);
extern void blendColor(float red, float green, float blue, float alpha);
extern void blendEquation(BlendEquationModeEXT mode);
extern void blendFuncSeparate(BlendingFactor sfactorRGB,
                              BlendingFactor dfactorRGB,
                              BlendingFactor sfactorAlpha,
                              BlendingFactor dfactorAlpha);
extern void
  multiDrawArrays(PrimitiveType mode, const int32_t* first, const core::SizeType* count, core::SizeType drawcount);
extern void multiDrawElements(PrimitiveType mode,
                              const core::SizeType* count,
                              DrawElementsType type,
                              const void* const* indices,
                              core::SizeType drawcount);
extern void pointParameter(PointParameterName pname, float param);
extern void pointParameter(PointParameterName pname, const float* params);
extern void pointParameter(PointParameterName pname, int32_t param);
extern void pointParameter(PointParameterName pname, const int32_t* params);
extern void beginQuery(QueryTarget target, uint32_t id);
extern void bindBuffer(BufferTarget target, uint32_t buffer);
extern void bufferData(BufferTarget target, std::size_t size, const void* data, BufferUsage usage);
extern void bufferSubData(BufferTarget target, std::intptr_t offset, std::size_t size, const void* data);
extern void deleteBuffers(core::SizeType n, const uint32_t* buffers);
extern void deleteQueries(core::SizeType n, const uint32_t* ids);
extern void endQuery(QueryTarget target);
extern void genBuffers(core::SizeType n, uint32_t* buffers);
extern void genQueries(core::SizeType n, uint32_t* ids);
extern void getBufferParameter(BufferTarget target, BufferPName pname, int32_t* params);
extern void getBufferPointer(BufferTarget target, BufferPointerName pname, void** params);
extern void getBufferSubData(BufferTarget target, std::intptr_t offset, std::size_t size, void* data);
extern void getQueryObject(uint32_t id, QueryObjectParameterName pname, int32_t* params);
extern void getQueryObject(uint32_t id, QueryObjectParameterName pname, uint32_t* params);
extern void getQuery(QueryTarget target, QueryParameterName pname, int32_t* params);
extern bool isBuffer(uint32_t buffer);
extern bool isQuery(uint32_t id);
extern void* mapBuffer(BufferTarget target, BufferAccess access);
extern bool unmapBuffer(BufferTarget target);
extern void attachShader(uint32_t program, uint32_t shader);
extern void bindAttribLocation(uint32_t program, uint32_t index, const char* name);
extern void blendEquationSeparate(BlendEquationModeEXT modeRGB, BlendEquationModeEXT modeAlpha);
extern void compileShader(uint32_t shader);
extern uint32_t createProgram();
extern uint32_t createShader(ShaderType type);
extern void deleteProgram(uint32_t program);
extern void deleteShader(uint32_t shader);
extern void detachShader(uint32_t program, uint32_t shader);
extern void disableVertexAttribArray(uint32_t index);
extern void drawBuffers(core::SizeType n, const DrawBufferMode* bufs);
extern void enableVertexAttribArray(uint32_t index);
extern void getActiveAttrib(uint32_t program,
                            uint32_t index,
                            core::SizeType bufSize,
                            core::SizeType* length,
                            int32_t* size,
                            AttributeType* type,
                            char* name);
extern void getActiveUniform(uint32_t program,
                             uint32_t index,
                             core::SizeType bufSize,
                             core::SizeType* length,
                             int32_t* size,
                             UniformType* type,
                             char* name);
extern void getAttachedShaders(uint32_t program, core::SizeType maxCount, core::SizeType* count, uint32_t* shaders);
extern int32_t getAttribLocation(uint32_t program, const char* name);
extern void getProgramInfoLog(uint32_t program, core::SizeType bufSize, core::SizeType* length, char* infoLog);
extern void getProgram(uint32_t program, ProgramProperty pname, int32_t* params);
extern void getShaderInfoLog(uint32_t shader, core::SizeType bufSize, core::SizeType* length, char* infoLog);
extern void getShaderSource(uint32_t shader, core::SizeType bufSize, core::SizeType* length, char* source);
extern void getShader(uint32_t shader, ShaderParameterName pname, int32_t* params);
extern int32_t getUniformLocation(uint32_t program, const char* name);
extern void getUniform(uint32_t program, int32_t location, float* params);
extern void getUniform(uint32_t program, int32_t location, int32_t* params);
extern void getVertexAttribPointer(uint32_t index, VertexAttribPointerProperty pname, void** pointer);
extern void getVertexAttrib(uint32_t index, VertexAttribProperty pname, double* params);
extern void getVertexAttrib(uint32_t index, VertexAttribProperty pname, float* params);
extern void getVertexAttrib(uint32_t index, VertexAttribProperty pname, int32_t* params);
extern bool isProgram(uint32_t program);
extern bool isShader(uint32_t shader);
extern void linkProgram(uint32_t program);
extern void shaderSource(uint32_t shader, core::SizeType count, const char* const* string, const int32_t* length);
extern void stencilFuncSeparate(TriangleFace face, StencilFunction func, int32_t ref, uint32_t mask);
extern void stencilMaskSeparate(TriangleFace face, uint32_t mask);
extern void stencilOpSeparate(TriangleFace face, StencilOp sfail, StencilOp dpfail, StencilOp dppass);
extern void uniform1(int32_t location, float v0);
extern void uniform1(int32_t location, core::SizeType count, const float* value);
extern void uniform1(int32_t location, int32_t v0);
extern void uniform1(int32_t location, core::SizeType count, const int32_t* value);
extern void uniform2(int32_t location, float v0, float v1);
extern void uniform2(int32_t location, core::SizeType count, const float* value);
extern void uniform2(int32_t location, int32_t v0, int32_t v1);
extern void uniform2(int32_t location, core::SizeType count, const int32_t* value);
extern void uniform3(int32_t location, float v0, float v1, float v2);
extern void uniform3(int32_t location, core::SizeType count, const float* value);
extern void uniform3(int32_t location, int32_t v0, int32_t v1, int32_t v2);
extern void uniform3(int32_t location, core::SizeType count, const int32_t* value);
extern void uniform4(int32_t location, float v0, float v1, float v2, float v3);
extern void uniform4(int32_t location, core::SizeType count, const float* value);
extern void uniform4(int32_t location, int32_t v0, int32_t v1, int32_t v2, int32_t v3);
extern void uniform4(int32_t location, core::SizeType count, const int32_t* value);
extern void uniformMatrix2(int32_t location, core::SizeType count, bool transpose, const float* value);
extern void uniformMatrix3(int32_t location, core::SizeType count, bool transpose, const float* value);
extern void uniformMatrix4(int32_t location, core::SizeType count, bool transpose, const float* value);
extern void useProgram(uint32_t program);
extern void validateProgram(uint32_t program);
extern void vertexAttrib1(uint32_t index, double x);
extern void vertexAttrib1(uint32_t index, const double* v);
extern void vertexAttrib1(uint32_t index, float x);
extern void vertexAttrib1(uint32_t index, const float* v);
extern void vertexAttrib1(uint32_t index, int16_t x);
extern void vertexAttrib1(uint32_t index, const int16_t* v);
extern void vertexAttrib2(uint32_t index, double x, double y);
extern void vertexAttrib2(uint32_t index, const double* v);
extern void vertexAttrib2(uint32_t index, float x, float y);
extern void vertexAttrib2(uint32_t index, const float* v);
extern void vertexAttrib2(uint32_t index, int16_t x, int16_t y);
extern void vertexAttrib2(uint32_t index, const int16_t* v);
extern void vertexAttrib3(uint32_t index, double x, double y, double z);
extern void vertexAttrib3(uint32_t index, const double* v);
extern void vertexAttrib3(uint32_t index, float x, float y, float z);
extern void vertexAttrib3(uint32_t index, const float* v);
extern void vertexAttrib3(uint32_t index, int16_t x, int16_t y, int16_t z);
extern void vertexAttrib3(uint32_t index, const int16_t* v);
extern void vertexAttrib4N(uint32_t index, const int8_t* v);
extern void vertexAttrib4N(uint32_t index, const int32_t* v);
extern void vertexAttrib4N(uint32_t index, const int16_t* v);
extern void vertexAttrib4N(uint32_t index, uint8_t x, uint8_t y, uint8_t z, uint8_t w);
extern void vertexAttrib4N(uint32_t index, const uint8_t* v);
extern void vertexAttrib4N(uint32_t index, const uint32_t* v);
extern void vertexAttrib4N(uint32_t index, const uint16_t* v);
extern void vertexAttrib4(uint32_t index, const int8_t* v);
extern void vertexAttrib4(uint32_t index, double x, double y, double z, double w);
extern void vertexAttrib4(uint32_t index, const double* v);
extern void vertexAttrib4(uint32_t index, float x, float y, float z, float w);
extern void vertexAttrib4(uint32_t index, const float* v);
extern void vertexAttrib4(uint32_t index, const int32_t* v);
extern void vertexAttrib4(uint32_t index, int16_t x, int16_t y, int16_t z, int16_t w);
extern void vertexAttrib4(uint32_t index, const int16_t* v);
extern void vertexAttrib4(uint32_t index, const uint8_t* v);
extern void vertexAttrib4(uint32_t index, const uint32_t* v);
extern void vertexAttrib4(uint32_t index, const uint16_t* v);
extern void vertexAttribPointer(uint32_t index,
                                int32_t size,
                                VertexAttribPointerType type,
                                bool normalized,
                                core::SizeType stride,
                                const void* pointer);
extern void uniformMatrix2x3(int32_t location, core::SizeType count, bool transpose, const float* value);
extern void uniformMatrix2x4(int32_t location, core::SizeType count, bool transpose, const float* value);
extern void uniformMatrix3x2(int32_t location, core::SizeType count, bool transpose, const float* value);
extern void uniformMatrix3x4(int32_t location, core::SizeType count, bool transpose, const float* value);
extern void uniformMatrix4x2(int32_t location, core::SizeType count, bool transpose, const float* value);
extern void uniformMatrix4x3(int32_t location, core::SizeType count, bool transpose, const float* value);
extern void beginConditionalRender(uint32_t id, ConditionalRenderMode mode);
extern void beginTransformFeedback(PrimitiveType primitiveMode);
extern void bindBufferBase(BufferTarget target, uint32_t index, uint32_t buffer);
extern void
  bindBufferRange(BufferTarget target, uint32_t index, uint32_t buffer, std::intptr_t offset, std::size_t size);
extern void bindFragDataLocation(uint32_t program, uint32_t color, const char* name);
extern void bindFramebuffer(FramebufferTarget target, uint32_t framebuffer);
extern void bindRenderbuffer(RenderbufferTarget target, uint32_t renderbuffer);
extern void bindVertexArray(uint32_t array);
extern void blitFramebuffer(int32_t srcX0,
                            int32_t srcY0,
                            int32_t srcX1,
                            int32_t srcY1,
                            int32_t dstX0,
                            int32_t dstY0,
                            int32_t dstX1,
                            int32_t dstY1,
                            core::Bitfield<ClearBufferMask> mask,
                            BlitFramebufferFilter filter);
extern FramebufferStatus checkFramebufferStatus(FramebufferTarget target);
extern void clampColor(ClampColorTarget target, ClampColorMode clamp);
extern void clearBufferf(Buffer buffer, int32_t drawbuffer, float depth, int32_t stencil);
extern void clearBuffer(Buffer buffer, int32_t drawbuffer, const float* value);
extern void clearBuffer(Buffer buffer, int32_t drawbuffer, const int32_t* value);
extern void clearBuffer(Buffer buffer, int32_t drawbuffer, const uint32_t* value);
extern void colorMask(uint32_t index, bool r, bool g, bool b, bool a);
extern void deleteFramebuffers(core::SizeType n, const uint32_t* framebuffers);
extern void deleteRenderbuffers(core::SizeType n, const uint32_t* renderbuffers);
extern void deleteVertexArrays(core::SizeType n, const uint32_t* arrays);
extern void disable(EnableCap target, uint32_t index);
extern void enable(EnableCap target, uint32_t index);
extern void endConditionalRender();
extern void endTransformFeedback();
extern void flushMappedBufferRange(BufferTarget target, std::intptr_t offset, std::size_t length);
extern void framebufferRenderbuffer(FramebufferTarget target,
                                    FramebufferAttachment attachment,
                                    RenderbufferTarget renderbuffertarget,
                                    uint32_t renderbuffer);
extern void framebufferTexture1D(
  FramebufferTarget target, FramebufferAttachment attachment, TextureTarget textarget, uint32_t texture, int32_t level);
extern void framebufferTexture2D(
  FramebufferTarget target, FramebufferAttachment attachment, TextureTarget textarget, uint32_t texture, int32_t level);
extern void framebufferTexture3D(FramebufferTarget target,
                                 FramebufferAttachment attachment,
                                 TextureTarget textarget,
                                 uint32_t texture,
                                 int32_t level,
                                 int32_t zoffset);
extern void framebufferTextureLayer(
  FramebufferTarget target, FramebufferAttachment attachment, uint32_t texture, int32_t level, int32_t layer);
extern void genFramebuffers(core::SizeType n, uint32_t* framebuffers);
extern void genRenderbuffers(core::SizeType n, uint32_t* renderbuffers);
extern void genVertexArrays(core::SizeType n, uint32_t* arrays);
extern void generateMipmap(TextureTarget target);
extern void getBooleanI(BufferTarget target, uint32_t index, bool* data);
extern int32_t getFragDataLocation(uint32_t program, const char* name);
extern void getFramebufferAttachmentParameter(FramebufferTarget target,
                                              FramebufferAttachment attachment,
                                              FramebufferAttachmentParameterName pname,
                                              int32_t* params);
extern void getIntegerI(GetPName target, uint32_t index, int32_t* data);
extern void getRenderbufferParameter(RenderbufferTarget target, RenderbufferParameterName pname, int32_t* params);
extern const uint8_t* getString(StringName name, uint32_t index);
extern void getTexParameterI(TextureTarget target, GetTextureParameter pname, int32_t* params);
extern void getTexParameterI(TextureTarget target, GetTextureParameter pname, uint32_t* params);
extern void getTransformFeedbackVarying(uint32_t program,
                                        uint32_t index,
                                        core::SizeType bufSize,
                                        core::SizeType* length,
                                        core::SizeType* size,
                                        AttributeType* type,
                                        char* name);
extern void getUniform(uint32_t program, int32_t location, uint32_t* params);
extern void getVertexAttribI(uint32_t index, VertexAttribEnum pname, int32_t* params);
extern void getVertexAttribI(uint32_t index, VertexAttribEnum pname, uint32_t* params);
extern bool isEnabled(EnableCap target, uint32_t index);
extern bool isFramebuffer(uint32_t framebuffer);
extern bool isRenderbuffer(uint32_t renderbuffer);
extern bool isVertexArray(uint32_t array);
extern void* mapBufferRange(BufferTarget target,
                            std::intptr_t offset,
                            std::size_t length,
                            core::Bitfield<MapBufferAccessMask> access);
extern void renderbufferStorage(RenderbufferTarget target,
                                InternalFormat internalformat,
                                core::SizeType width,
                                core::SizeType height);
extern void renderbufferStorageMultisample(RenderbufferTarget target,
                                           core::SizeType samples,
                                           InternalFormat internalformat,
                                           core::SizeType width,
                                           core::SizeType height);
extern void texParameterI(TextureTarget target, TextureParameterName pname, const int32_t* params);
extern void texParameterI(TextureTarget target, TextureParameterName pname, const uint32_t* params);
extern void transformFeedbackVarying(uint32_t program,
                                     core::SizeType count,
                                     const char* const* varyings,
                                     TransformFeedbackBufferMode bufferMode);
extern void uniform1(int32_t location, uint32_t v0);
extern void uniform1(int32_t location, core::SizeType count, const uint32_t* value);
extern void uniform2(int32_t location, uint32_t v0, uint32_t v1);
extern void uniform2(int32_t location, core::SizeType count, const uint32_t* value);
extern void uniform3(int32_t location, uint32_t v0, uint32_t v1, uint32_t v2);
extern void uniform3(int32_t location, core::SizeType count, const uint32_t* value);
extern void uniform4(int32_t location, uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3);
extern void uniform4(int32_t location, core::SizeType count, const uint32_t* value);
extern void vertexAttribI1(uint32_t index, int32_t x);
extern void vertexAttribI1(uint32_t index, const int32_t* v);
extern void vertexAttribI1(uint32_t index, uint32_t x);
extern void vertexAttribI1(uint32_t index, const uint32_t* v);
extern void vertexAttribI2(uint32_t index, int32_t x, int32_t y);
extern void vertexAttribI2(uint32_t index, const int32_t* v);
extern void vertexAttribI2(uint32_t index, uint32_t x, uint32_t y);
extern void vertexAttribI2(uint32_t index, const uint32_t* v);
extern void vertexAttribI3(uint32_t index, int32_t x, int32_t y, int32_t z);
extern void vertexAttribI3(uint32_t index, const int32_t* v);
extern void vertexAttribI3(uint32_t index, uint32_t x, uint32_t y, uint32_t z);
extern void vertexAttribI3(uint32_t index, const uint32_t* v);
extern void vertexAttribI4(uint32_t index, const int8_t* v);
extern void vertexAttribI4(uint32_t index, int32_t x, int32_t y, int32_t z, int32_t w);
extern void vertexAttribI4(uint32_t index, const int32_t* v);
extern void vertexAttribI4(uint32_t index, const int16_t* v);
extern void vertexAttribI4(uint32_t index, const uint8_t* v);
extern void vertexAttribI4(uint32_t index, uint32_t x, uint32_t y, uint32_t z, uint32_t w);
extern void vertexAttribI4(uint32_t index, const uint32_t* v);
extern void vertexAttribI4(uint32_t index, const uint16_t* v);
extern void vertexAttribIPointer(
  uint32_t index, int32_t size, VertexAttribIType type, core::SizeType stride, const void* pointer);
extern void copyBufferSubData(CopyBufferSubDataTarget readTarget,
                              CopyBufferSubDataTarget writeTarget,
                              std::intptr_t readOffset,
                              std::intptr_t writeOffset,
                              std::size_t size);
extern void drawArraysInstanced(PrimitiveType mode, int32_t first, core::SizeType count, core::SizeType instancecount);
extern void drawElementsInstanced(
  PrimitiveType mode, core::SizeType count, DrawElementsType type, const void* indices, core::SizeType instancecount);
extern void getActiveUniformBlockName(
  uint32_t program, uint32_t uniformBlockIndex, core::SizeType bufSize, core::SizeType* length, char* uniformBlockName);
extern void
  getActiveUniformBlock(uint32_t program, uint32_t uniformBlockIndex, UniformBlockPName pname, int32_t* params);
extern void getActiveUniformName(
  uint32_t program, uint32_t uniformIndex, core::SizeType bufSize, core::SizeType* length, char* uniformName);
extern void getActiveUniforms(
  uint32_t program, core::SizeType uniformCount, const uint32_t* uniformIndices, UniformPName pname, int32_t* params);
extern uint32_t getUniformBlockIndex(uint32_t program, const char* uniformBlockName);
extern void getUniformIndice(uint32_t program,
                             core::SizeType uniformCount,
                             const char* const* uniformNames,
                             uint32_t* uniformIndices);
extern void primitiveRestartIndex(uint32_t index);
extern void texBuffer(TextureTarget target, SizedInternalFormat internalformat, uint32_t buffer);
extern void uniformBlockBinding(uint32_t program, uint32_t uniformBlockIndex, uint32_t uniformBlockBinding);
extern void drawElementsBaseVertex(
  PrimitiveType mode, core::SizeType count, DrawElementsType type, const void* indices, int32_t basevertex);
extern void drawElementsInstancedBaseVertex(PrimitiveType mode,
                                            core::SizeType count,
                                            DrawElementsType type,
                                            const void* indices,
                                            core::SizeType instancecount,
                                            int32_t basevertex);
extern void drawRangeElementsBaseVertex(PrimitiveType mode,
                                        uint32_t start,
                                        uint32_t end,
                                        core::SizeType count,
                                        DrawElementsType type,
                                        const void* indices,
                                        int32_t basevertex);
extern void
  framebufferTexture(FramebufferTarget target, FramebufferAttachment attachment, uint32_t texture, int32_t level);
extern void getBufferParameter(BufferTarget target, BufferPName pname, int64_t* params);
extern void getInteger64I(GetPName target, uint32_t index, int64_t* data);
extern void getMultisample(GetMultisamplePNameNV pname, uint32_t index, float* val);
extern void multiDrawElementsBaseVertex(PrimitiveType mode,
                                        const core::SizeType* count,
                                        DrawElementsType type,
                                        const void* const* indices,
                                        core::SizeType drawcount,
                                        const int32_t* basevertex);
extern void provokingVertex(VertexProvokingMode mode);
extern void sampleMask(uint32_t maskNumber, uint32_t mask);
extern void texImage2DMultisample(TextureTarget target,
                                  core::SizeType samples,
                                  InternalFormat internalformat,
                                  core::SizeType width,
                                  core::SizeType height,
                                  bool fixedsamplelocations);
extern void texImage3DMultisample(TextureTarget target,
                                  core::SizeType samples,
                                  InternalFormat internalformat,
                                  core::SizeType width,
                                  core::SizeType height,
                                  core::SizeType depth,
                                  bool fixedsamplelocations);
extern void bindFragDataLocationIndexed(uint32_t program, uint32_t colorNumber, uint32_t index, const char* name);
extern void bindSampler(uint32_t unit, uint32_t sampler);
extern void deleteSampler(core::SizeType count, const uint32_t* samplers);
extern void genSampler(core::SizeType count, uint32_t* samplers);
extern int32_t getFragDataIndex(uint32_t program, const char* name);
extern void getQueryObject(uint32_t id, QueryObjectParameterName pname, int64_t* params);
extern void getQueryObject(uint32_t id, QueryObjectParameterName pname, uint64_t* params);
extern void getSamplerParameterI(uint32_t sampler, SamplerParameterI pname, int32_t* params);
extern void getSamplerParameterI(uint32_t sampler, SamplerParameterI pname, uint32_t* params);
extern void getSamplerParameter(uint32_t sampler, SamplerParameterF pname, float* params);
extern void getSamplerParameter(uint32_t sampler, SamplerParameterI pname, int32_t* params);
extern bool isSampler(uint32_t sampler);
extern void queryCounter(uint32_t id, QueryCounterTarget target);
extern void samplerParameterI(uint32_t sampler, SamplerParameterI pname, const int32_t* param);
extern void samplerParameterI(uint32_t sampler, SamplerParameterI pname, const uint32_t* param);
extern void samplerParameter(uint32_t sampler, SamplerParameterF pname, float param);
extern void samplerParameter(uint32_t sampler, SamplerParameterF pname, const float* param);
extern void samplerParameter(uint32_t sampler, SamplerParameterI pname, int32_t param);
extern void samplerParameter(uint32_t sampler, SamplerParameterI pname, const int32_t* param);
extern void vertexAttribDivisor(uint32_t index, uint32_t divisor);
extern void vertexAttribP1(uint32_t index, VertexAttribPointerType type, bool normalized, uint32_t value);
extern void vertexAttribP1(uint32_t index, VertexAttribPointerType type, bool normalized, const uint32_t* value);
extern void vertexAttribP2(uint32_t index, VertexAttribPointerType type, bool normalized, uint32_t value);
extern void vertexAttribP2(uint32_t index, VertexAttribPointerType type, bool normalized, const uint32_t* value);
extern void vertexAttribP3(uint32_t index, VertexAttribPointerType type, bool normalized, uint32_t value);
extern void vertexAttribP3(uint32_t index, VertexAttribPointerType type, bool normalized, const uint32_t* value);
extern void vertexAttribP4(uint32_t index, VertexAttribPointerType type, bool normalized, uint32_t value);
extern void vertexAttribP4(uint32_t index, VertexAttribPointerType type, bool normalized, const uint32_t* value);
extern void beginQueryIndexed(QueryTarget target, uint32_t index, uint32_t id);
extern void bindTransformFeedback(BindTransformFeedbackTarget target, uint32_t id);
extern void blendEquationSeparate(uint32_t buf, BlendEquationModeEXT modeRGB, BlendEquationModeEXT modeAlpha);
extern void blendEquation(uint32_t buf, BlendEquationModeEXT mode);
extern void blendFuncSeparate(
  uint32_t buf, BlendingFactor srcRGB, BlendingFactor dstRGB, BlendingFactor srcAlpha, BlendingFactor dstAlpha);
extern void blendFunc(uint32_t buf, BlendingFactor src, BlendingFactor dst);
extern void deleteTransformFeedback(core::SizeType n, const uint32_t* ids);
extern void drawArraysIndirect(PrimitiveType mode, const void* indirect);
extern void drawElementsIndirect(PrimitiveType mode, DrawElementsType type, const void* indirect);
extern void drawTransformFeedback(PrimitiveType mode, uint32_t id);
extern void drawTransformFeedbackStream(PrimitiveType mode, uint32_t id, uint32_t stream);
extern void endQueryIndexed(QueryTarget target, uint32_t index);
extern void genTransformFeedback(core::SizeType n, uint32_t* ids);
extern void getActiveSubroutineName(
  uint32_t program, ShaderType shadertype, uint32_t index, core::SizeType bufSize, core::SizeType* length, char* name);
extern void getActiveSubroutineUniformName(
  uint32_t program, ShaderType shadertype, uint32_t index, core::SizeType bufSize, core::SizeType* length, char* name);
extern void getActiveSubroutineUniform(
  uint32_t program, ShaderType shadertype, uint32_t index, SubroutineParameterName pname, int32_t* values);
extern void getProgramStage(uint32_t program, ShaderType shadertype, ProgramStagePName pname, int32_t* values);
extern void getQueryIndexed(QueryTarget target, uint32_t index, QueryParameterName pname, int32_t* params);
extern uint32_t getSubroutineIndex(uint32_t program, ShaderType shadertype, const char* name);
extern int32_t getSubroutineUniformLocation(uint32_t program, ShaderType shadertype, const char* name);
extern void getUniformSubroutine(ShaderType shadertype, int32_t location, uint32_t* params);
extern void getUniform(uint32_t program, int32_t location, double* params);
extern bool isTransformFeedback(uint32_t id);
extern void minSampleShading(float value);
extern void patchParameter(PatchParameterName pname, const float* values);
extern void patchParameter(PatchParameterName pname, int32_t value);
extern void pauseTransformFeedback();
extern void resumeTransformFeedback();
extern void uniform1(int32_t location, double x);
extern void uniform1(int32_t location, core::SizeType count, const double* value);
extern void uniform2(int32_t location, double x, double y);
extern void uniform2(int32_t location, core::SizeType count, const double* value);
extern void uniform3(int32_t location, double x, double y, double z);
extern void uniform3(int32_t location, core::SizeType count, const double* value);
extern void uniform4(int32_t location, double x, double y, double z, double w);
extern void uniform4(int32_t location, core::SizeType count, const double* value);
extern void uniformMatrix2(int32_t location, core::SizeType count, bool transpose, const double* value);
extern void uniformMatrix2x3(int32_t location, core::SizeType count, bool transpose, const double* value);
extern void uniformMatrix2x4(int32_t location, core::SizeType count, bool transpose, const double* value);
extern void uniformMatrix3(int32_t location, core::SizeType count, bool transpose, const double* value);
extern void uniformMatrix3x2(int32_t location, core::SizeType count, bool transpose, const double* value);
extern void uniformMatrix3x4(int32_t location, core::SizeType count, bool transpose, const double* value);
extern void uniformMatrix4(int32_t location, core::SizeType count, bool transpose, const double* value);
extern void uniformMatrix4x2(int32_t location, core::SizeType count, bool transpose, const double* value);
extern void uniformMatrix4x3(int32_t location, core::SizeType count, bool transpose, const double* value);
extern void uniformSubroutines(ShaderType shadertype, core::SizeType count, const uint32_t* indices);
extern void activeShaderProgram(uint32_t pipeline, uint32_t program);
extern void bindProgramPipeline(uint32_t pipeline);
extern void clearDepth(float d);
extern uint32_t createShaderProgram(ShaderType type, core::SizeType count, const char* const* strings);
extern void deleteProgramPipeline(core::SizeType n, const uint32_t* pipelines);
extern void depthRangeArray(uint32_t first, core::SizeType count, const double* v);
extern void depthRangeIndexed(uint32_t index, double n, double f);
extern void depthRange(float n, float f);
extern void genProgramPipeline(core::SizeType n, uint32_t* pipelines);
extern void getDoubleI(GetPName target, uint32_t index, double* data);
extern void getFloatI(GetPName target, uint32_t index, float* data);
extern void getProgramBinary(
  uint32_t program, core::SizeType bufSize, core::SizeType* length, core::EnumType* binaryFormat, void* binary);
extern void getProgramPipelineInfoLog(uint32_t pipeline, core::SizeType bufSize, core::SizeType* length, char* infoLog);
extern void getProgramPipeline(uint32_t pipeline, PipelineParameterName pname, int32_t* params);
extern void
  getShaderPrecisionFormat(ShaderType shadertype, PrecisionType precisiontype, int32_t* range, int32_t* precision);
extern void getVertexAttribL(uint32_t index, VertexAttribEnum pname, double* params);
extern bool isProgramPipeline(uint32_t pipeline);
extern void programBinary(uint32_t program, core::EnumType binaryFormat, const void* binary, core::SizeType length);
extern void programParameter(uint32_t program, ProgramParameterPName pname, int32_t value);
extern void programUniform1(uint32_t program, int32_t location, double v0);
extern void programUniform1(uint32_t program, int32_t location, core::SizeType count, const double* value);
extern void programUniform1(uint32_t program, int32_t location, float v0);
extern void programUniform1(uint32_t program, int32_t location, core::SizeType count, const float* value);
extern void programUniform1(uint32_t program, int32_t location, int32_t v0);
extern void programUniform1(uint32_t program, int32_t location, core::SizeType count, const int32_t* value);
extern void programUniform1(uint32_t program, int32_t location, uint32_t v0);
extern void programUniform1(uint32_t program, int32_t location, core::SizeType count, const uint32_t* value);
extern void programUniform2(uint32_t program, int32_t location, double v0, double v1);
extern void programUniform2(uint32_t program, int32_t location, core::SizeType count, const double* value);
extern void programUniform2(uint32_t program, int32_t location, float v0, float v1);
extern void programUniform2(uint32_t program, int32_t location, core::SizeType count, const float* value);
extern void programUniform2(uint32_t program, int32_t location, int32_t v0, int32_t v1);
extern void programUniform2(uint32_t program, int32_t location, core::SizeType count, const int32_t* value);
extern void programUniform2(uint32_t program, int32_t location, uint32_t v0, uint32_t v1);
extern void programUniform2(uint32_t program, int32_t location, core::SizeType count, const uint32_t* value);
extern void programUniform3(uint32_t program, int32_t location, double v0, double v1, double v2);
extern void programUniform3(uint32_t program, int32_t location, core::SizeType count, const double* value);
extern void programUniform3(uint32_t program, int32_t location, float v0, float v1, float v2);
extern void programUniform3(uint32_t program, int32_t location, core::SizeType count, const float* value);
extern void programUniform3(uint32_t program, int32_t location, int32_t v0, int32_t v1, int32_t v2);
extern void programUniform3(uint32_t program, int32_t location, core::SizeType count, const int32_t* value);
extern void programUniform3(uint32_t program, int32_t location, uint32_t v0, uint32_t v1, uint32_t v2);
extern void programUniform3(uint32_t program, int32_t location, core::SizeType count, const uint32_t* value);
extern void programUniform4(uint32_t program, int32_t location, double v0, double v1, double v2, double v3);
extern void programUniform4(uint32_t program, int32_t location, core::SizeType count, const double* value);
extern void programUniform4(uint32_t program, int32_t location, float v0, float v1, float v2, float v3);
extern void programUniform4(uint32_t program, int32_t location, core::SizeType count, const float* value);
extern void programUniform4(uint32_t program, int32_t location, int32_t v0, int32_t v1, int32_t v2, int32_t v3);
extern void programUniform4(uint32_t program, int32_t location, core::SizeType count, const int32_t* value);
extern void programUniform4(uint32_t program, int32_t location, uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3);
extern void programUniform4(uint32_t program, int32_t location, core::SizeType count, const uint32_t* value);
extern void
  programUniformMatrix2(uint32_t program, int32_t location, core::SizeType count, bool transpose, const double* value);
extern void
  programUniformMatrix2(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float* value);
extern void programUniformMatrix2x3(
  uint32_t program, int32_t location, core::SizeType count, bool transpose, const double* value);
extern void
  programUniformMatrix2x3(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float* value);
extern void programUniformMatrix2x4(
  uint32_t program, int32_t location, core::SizeType count, bool transpose, const double* value);
extern void
  programUniformMatrix2x4(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float* value);
extern void
  programUniformMatrix3(uint32_t program, int32_t location, core::SizeType count, bool transpose, const double* value);
extern void
  programUniformMatrix3(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float* value);
extern void programUniformMatrix3x2(
  uint32_t program, int32_t location, core::SizeType count, bool transpose, const double* value);
extern void
  programUniformMatrix3x2(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float* value);
extern void programUniformMatrix3x4(
  uint32_t program, int32_t location, core::SizeType count, bool transpose, const double* value);
extern void
  programUniformMatrix3x4(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float* value);
extern void
  programUniformMatrix4(uint32_t program, int32_t location, core::SizeType count, bool transpose, const double* value);
extern void
  programUniformMatrix4(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float* value);
extern void programUniformMatrix4x2(
  uint32_t program, int32_t location, core::SizeType count, bool transpose, const double* value);
extern void
  programUniformMatrix4x2(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float* value);
extern void programUniformMatrix4x3(
  uint32_t program, int32_t location, core::SizeType count, bool transpose, const double* value);
extern void
  programUniformMatrix4x3(uint32_t program, int32_t location, core::SizeType count, bool transpose, const float* value);
extern void releaseShaderCompiler();
extern void scissorArray(uint32_t first, core::SizeType count, const int32_t* v);
extern void scissorIndexed(uint32_t index, int32_t left, int32_t bottom, core::SizeType width, core::SizeType height);
extern void scissorIndexed(uint32_t index, const int32_t* v);
extern void useProgramStage(uint32_t pipeline, core::Bitfield<UseProgramStageMask> stages, uint32_t program);
extern void validateProgramPipeline(uint32_t pipeline);
extern void vertexAttribL1(uint32_t index, double x);
extern void vertexAttribL1(uint32_t index, const double* v);
extern void vertexAttribL2(uint32_t index, double x, double y);
extern void vertexAttribL2(uint32_t index, const double* v);
extern void vertexAttribL3(uint32_t index, double x, double y, double z);
extern void vertexAttribL3(uint32_t index, const double* v);
extern void vertexAttribL4(uint32_t index, double x, double y, double z, double w);
extern void vertexAttribL4(uint32_t index, const double* v);
extern void vertexAttribLPointer(
  uint32_t index, int32_t size, VertexAttribLType type, core::SizeType stride, const void* pointer);
extern void viewportArray(uint32_t first, core::SizeType count, const float* v);
extern void viewportIndexed(uint32_t index, float x, float y, float w, float h);
extern void viewportIndexed(uint32_t index, const float* v);
extern void bindImageTexture(uint32_t unit,
                             uint32_t texture,
                             int32_t level,
                             bool layered,
                             int32_t layer,
                             BufferAccess access,
                             InternalFormat format);
extern void drawArraysInstancedBaseInstance(
  PrimitiveType mode, int32_t first, core::SizeType count, core::SizeType instancecount, uint32_t baseinstance);
extern void drawElementsInstancedBaseInstance(PrimitiveType mode,
                                              core::SizeType count,
                                              DrawElementsType type,
                                              const void* indices,
                                              core::SizeType instancecount,
                                              uint32_t baseinstance);
extern void drawElementsInstancedBaseVertexBaseInstance(PrimitiveType mode,
                                                        core::SizeType count,
                                                        DrawElementsType type,
                                                        const void* indices,
                                                        core::SizeType instancecount,
                                                        int32_t basevertex,
                                                        uint32_t baseinstance);
extern void drawTransformFeedbackInstanced(PrimitiveType mode, uint32_t id, core::SizeType instancecount);
extern void
  drawTransformFeedbackStreamInstanced(PrimitiveType mode, uint32_t id, uint32_t stream, core::SizeType instancecount);
extern void
  getActiveAtomicCounterBuffer(uint32_t program, uint32_t bufferIndex, AtomicCounterBufferPName pname, int32_t* params);
extern void getInternalformat(TextureTarget target,
                              InternalFormat internalformat,
                              InternalFormatPName pname,
                              core::SizeType count,
                              int32_t* params);
extern void memoryBarrier(core::Bitfield<MemoryBarrierMask> barriers);
extern void
  texStorage1D(TextureTarget target, core::SizeType levels, SizedInternalFormat internalformat, core::SizeType width);
extern void texStorage2D(TextureTarget target,
                         core::SizeType levels,
                         SizedInternalFormat internalformat,
                         core::SizeType width,
                         core::SizeType height);
extern void texStorage3D(TextureTarget target,
                         core::SizeType levels,
                         SizedInternalFormat internalformat,
                         core::SizeType width,
                         core::SizeType height,
                         core::SizeType depth);
extern void bindVertexBuffer(uint32_t bindingindex, uint32_t buffer, std::intptr_t offset, core::SizeType stride);
extern void clearBufferData(
  BufferStorageTarget target, SizedInternalFormat internalformat, PixelFormat format, PixelType type, const void* data);
extern void clearBufferSubData(BufferTarget target,
                               SizedInternalFormat internalformat,
                               std::intptr_t offset,
                               std::size_t size,
                               PixelFormat format,
                               PixelType type,
                               const void* data);
extern void copyImageSubData(uint32_t srcName,
                             CopyImageSubDataTarget srcTarget,
                             int32_t srcLevel,
                             int32_t srcX,
                             int32_t srcY,
                             int32_t srcZ,
                             uint32_t dstName,
                             CopyImageSubDataTarget dstTarget,
                             int32_t dstLevel,
                             int32_t dstX,
                             int32_t dstY,
                             int32_t dstZ,
                             core::SizeType srcWidth,
                             core::SizeType srcHeight,
                             core::SizeType srcDepth);
extern void debugMessageCallback(core::DebugProc callback, const void* userParam);
extern void debugMessageControl(
  DebugSource source, DebugType type, DebugSeverity severity, core::SizeType count, const uint32_t* ids, bool enabled);
extern void debugMessageInsert(
  DebugSource source, DebugType type, uint32_t id, DebugSeverity severity, core::SizeType length, const char* buf);
extern void dispatchCompute(uint32_t num_groups_x, uint32_t num_groups_y, uint32_t num_groups_z);
extern void dispatchComputeIndirect(std::intptr_t indirect);
extern void framebufferParameter(FramebufferTarget target, FramebufferParameterName pname, int32_t param);
extern uint32_t getDebugMessageLog(uint32_t count,
                                   core::SizeType bufSize,
                                   DebugSource* sources,
                                   DebugType* types,
                                   uint32_t* ids,
                                   DebugSeverity* severities,
                                   core::SizeType* lengths,
                                   char* messageLog);
extern void
  getFramebufferParameter(FramebufferTarget target, FramebufferAttachmentParameterName pname, int32_t* params);
extern void getInternalformat(TextureTarget target,
                              InternalFormat internalformat,
                              InternalFormatPName pname,
                              core::SizeType count,
                              int64_t* params);
extern void getObjectLabel(
  ObjectIdentifier identifier, uint32_t name, core::SizeType bufSize, core::SizeType* length, char* label);
extern void getObjectPtrLabel(const void* ptr, core::SizeType bufSize, core::SizeType* length, char* label);
extern void getProgramInterface(uint32_t program,
                                ProgramInterface programInterface,
                                ProgramInterfacePName pname,
                                int32_t* params);
extern uint32_t getProgramResourceIndex(uint32_t program, ProgramInterface programInterface, const char* name);
extern int32_t getProgramResourceLocation(uint32_t program, ProgramInterface programInterface, const char* name);
extern int32_t getProgramResourceLocationIndex(uint32_t program, ProgramInterface programInterface, const char* name);
extern void getProgramResourceName(uint32_t program,
                                   ProgramInterface programInterface,
                                   uint32_t index,
                                   core::SizeType bufSize,
                                   core::SizeType* length,
                                   char* name);
extern void getProgramResource(uint32_t program,
                               ProgramInterface programInterface,
                               uint32_t index,
                               core::SizeType propCount,
                               const ProgramResourceProperty* props,
                               core::SizeType count,
                               core::SizeType* length,
                               int32_t* params);
extern void invalidateBufferData(uint32_t buffer);
extern void invalidateBufferSubData(uint32_t buffer, std::intptr_t offset, std::size_t length);
extern void invalidateFramebuffer(FramebufferTarget target,
                                  core::SizeType numAttachments,
                                  const InvalidateFramebufferAttachment* attachments);
extern void invalidateSubFramebuffer(FramebufferTarget target,
                                     core::SizeType numAttachments,
                                     const InvalidateFramebufferAttachment* attachments,
                                     int32_t x,
                                     int32_t y,
                                     core::SizeType width,
                                     core::SizeType height);
extern void invalidateTexImage(uint32_t texture, int32_t level);
extern void invalidateTexSubImage(uint32_t texture,
                                  int32_t level,
                                  int32_t xoffset,
                                  int32_t yoffset,
                                  int32_t zoffset,
                                  core::SizeType width,
                                  core::SizeType height,
                                  core::SizeType depth);
extern void
  multiDrawArraysIndirect(PrimitiveType mode, const void* indirect, core::SizeType drawcount, core::SizeType stride);
extern void multiDrawElementsIndirect(
  PrimitiveType mode, DrawElementsType type, const void* indirect, core::SizeType drawcount, core::SizeType stride);
extern void objectLabel(ObjectIdentifier identifier, uint32_t name, core::SizeType length, const char* label);
extern void objectPtrLabel(const void* ptr, core::SizeType length, const char* label);
extern void popDebugGroup();
extern void pushDebugGroup(DebugSource source, uint32_t id, core::SizeType length, const char* message);
extern void shaderStorageBlockBinding(uint32_t program, uint32_t storageBlockIndex, uint32_t storageBlockBinding);
extern void texBufferRange(
  TextureTarget target, SizedInternalFormat internalformat, uint32_t buffer, std::intptr_t offset, std::size_t size);
extern void texStorage2DMultisample(TextureTarget target,
                                    core::SizeType samples,
                                    SizedInternalFormat internalformat,
                                    core::SizeType width,
                                    core::SizeType height,
                                    bool fixedsamplelocations);
extern void texStorage3DMultisample(TextureTarget target,
                                    core::SizeType samples,
                                    SizedInternalFormat internalformat,
                                    core::SizeType width,
                                    core::SizeType height,
                                    core::SizeType depth,
                                    bool fixedsamplelocations);
extern void textureView(uint32_t texture,
                        TextureTarget target,
                        uint32_t origtexture,
                        SizedInternalFormat internalformat,
                        uint32_t minlevel,
                        uint32_t numlevels,
                        uint32_t minlayer,
                        uint32_t numlayers);
extern void vertexAttribBinding(uint32_t attribindex, uint32_t bindingindex);
extern void vertexAttribFormat(
  uint32_t attribindex, int32_t size, VertexAttribType type, bool normalized, uint32_t relativeoffset);
extern void vertexAttribIFormat(uint32_t attribindex, int32_t size, VertexAttribIType type, uint32_t relativeoffset);
extern void vertexAttribLFormat(uint32_t attribindex, int32_t size, VertexAttribLType type, uint32_t relativeoffset);
extern void vertexBindingDivisor(uint32_t bindingindex, uint32_t divisor);
extern void bindBuffersBase(BufferTarget target, uint32_t first, core::SizeType count, const uint32_t* buffers);
extern void bindBuffersRange(BufferTarget target,
                             uint32_t first,
                             core::SizeType count,
                             const uint32_t* buffers,
                             const std::intptr_t* offsets,
                             const std::size_t* sizes);
extern void bindImageTextures(uint32_t first, core::SizeType count, const uint32_t* textures);
extern void bindSampler(uint32_t first, core::SizeType count, const uint32_t* samplers);
extern void bindTextures(uint32_t first, core::SizeType count, const uint32_t* textures);
extern void bindVertexBuffers(uint32_t first,
                              core::SizeType count,
                              const uint32_t* buffers,
                              const std::intptr_t* offsets,
                              const core::SizeType* strides);
extern void bufferStorage(BufferStorageTarget target,
                          std::size_t size,
                          const void* data,
                          core::Bitfield<BufferStorageMask> flags);
extern void clearTexImage(uint32_t texture, int32_t level, PixelFormat format, PixelType type, const void* data);
extern void clearTexSubImage(uint32_t texture,
                             int32_t level,
                             int32_t xoffset,
                             int32_t yoffset,
                             int32_t zoffset,
                             core::SizeType width,
                             core::SizeType height,
                             core::SizeType depth,
                             PixelFormat format,
                             PixelType type,
                             const void* data);
extern void bindTextureUnit(uint32_t unit, uint32_t texture);
extern void blitNamedFramebuffer(uint32_t readFramebuffer,
                                 uint32_t drawFramebuffer,
                                 int32_t srcX0,
                                 int32_t srcY0,
                                 int32_t srcX1,
                                 int32_t srcY1,
                                 int32_t dstX0,
                                 int32_t dstY0,
                                 int32_t dstX1,
                                 int32_t dstY1,
                                 core::Bitfield<ClearBufferMask> mask,
                                 BlitFramebufferFilter filter);
extern FramebufferStatus checkNamedFramebufferStatus(uint32_t framebuffer, FramebufferTarget target);
extern void clearNamedBufferData(
  uint32_t buffer, SizedInternalFormat internalformat, PixelFormat format, PixelType type, const void* data);
extern void clearNamedBufferSubData(uint32_t buffer,
                                    SizedInternalFormat internalformat,
                                    std::intptr_t offset,
                                    std::size_t size,
                                    PixelFormat format,
                                    PixelType type,
                                    const void* data);
extern void
  clearNamedFramebufferf(uint32_t framebuffer, Buffer buffer, int32_t drawbuffer, float depth, int32_t stencil);
extern void clearNamedFramebuffer(uint32_t framebuffer, Buffer buffer, int32_t drawbuffer, const float* value);
extern void clearNamedFramebuffer(uint32_t framebuffer, Buffer buffer, int32_t drawbuffer, const int32_t* value);
extern void clearNamedFramebuffer(uint32_t framebuffer, Buffer buffer, int32_t drawbuffer, const uint32_t* value);
extern void clipControl(ClipControlOrigin origin, ClipControlDepth depth);
extern void compressedTextureSubImage1D(uint32_t texture,
                                        int32_t level,
                                        int32_t xoffset,
                                        core::SizeType width,
                                        InternalFormat format,
                                        core::SizeType imageSize,
                                        const void* data);
extern void compressedTextureSubImage2D(uint32_t texture,
                                        int32_t level,
                                        int32_t xoffset,
                                        int32_t yoffset,
                                        core::SizeType width,
                                        core::SizeType height,
                                        InternalFormat format,
                                        core::SizeType imageSize,
                                        const void* data);
extern void compressedTextureSubImage3D(uint32_t texture,
                                        int32_t level,
                                        int32_t xoffset,
                                        int32_t yoffset,
                                        int32_t zoffset,
                                        core::SizeType width,
                                        core::SizeType height,
                                        core::SizeType depth,
                                        InternalFormat format,
                                        core::SizeType imageSize,
                                        const void* data);
extern void copyNamedBufferSubData(
  uint32_t readBuffer, uint32_t writeBuffer, std::intptr_t readOffset, std::intptr_t writeOffset, std::size_t size);
extern void
  copyTextureSubImage1D(uint32_t texture, int32_t level, int32_t xoffset, int32_t x, int32_t y, core::SizeType width);
extern void copyTextureSubImage2D(uint32_t texture,
                                  int32_t level,
                                  int32_t xoffset,
                                  int32_t yoffset,
                                  int32_t x,
                                  int32_t y,
                                  core::SizeType width,
                                  core::SizeType height);
extern void copyTextureSubImage3D(uint32_t texture,
                                  int32_t level,
                                  int32_t xoffset,
                                  int32_t yoffset,
                                  int32_t zoffset,
                                  int32_t x,
                                  int32_t y,
                                  core::SizeType width,
                                  core::SizeType height);
extern void createBuffers(core::SizeType n, uint32_t* buffers);
extern void createFramebuffers(core::SizeType n, uint32_t* framebuffers);
extern void createProgramPipeline(core::SizeType n, uint32_t* pipelines);
extern void createQueries(QueryTarget target, core::SizeType n, uint32_t* ids);
extern void createRenderbuffers(core::SizeType n, uint32_t* renderbuffers);
extern void createSampler(core::SizeType n, uint32_t* samplers);
extern void createTextures(TextureTarget target, core::SizeType n, uint32_t* textures);
extern void createTransformFeedback(core::SizeType n, uint32_t* ids);
extern void createVertexArrays(core::SizeType n, uint32_t* arrays);
extern void disableVertexArrayAttrib(uint32_t vaobj, uint32_t index);
extern void enableVertexArrayAttrib(uint32_t vaobj, uint32_t index);
extern void flushMappedNamedBufferRange(uint32_t buffer, std::intptr_t offset, std::size_t length);
extern void generateTextureMipmap(uint32_t texture);
extern void getCompressedTextureImage(uint32_t texture, int32_t level, core::SizeType bufSize, void* pixels);
extern void getCompressedTextureSubImage(uint32_t texture,
                                         int32_t level,
                                         int32_t xoffset,
                                         int32_t yoffset,
                                         int32_t zoffset,
                                         core::SizeType width,
                                         core::SizeType height,
                                         core::SizeType depth,
                                         core::SizeType bufSize,
                                         void* pixels);
extern GraphicsResetStatus getGraphicsResetStatus();
extern void getNamedBufferParameter(uint32_t buffer, BufferPName pname, int64_t* params);
extern void getNamedBufferParameter(uint32_t buffer, BufferPName pname, int32_t* params);
extern void getNamedBufferPointer(uint32_t buffer, BufferPointerName pname, void** params);
extern void getNamedBufferSubData(uint32_t buffer, std::intptr_t offset, std::size_t size, void* data);
extern void getNamedFramebufferAttachmentParameter(uint32_t framebuffer,
                                                   FramebufferAttachment attachment,
                                                   FramebufferAttachmentParameterName pname,
                                                   int32_t* params);
extern void getNamedFramebufferParameter(uint32_t framebuffer, GetFramebufferParameter pname, int32_t* param);
extern void getNamedRenderbufferParameter(uint32_t renderbuffer, RenderbufferParameterName pname, int32_t* params);
extern void
  getQueryBufferObjecti64v(uint32_t id, uint32_t buffer, QueryObjectParameterName pname, std::intptr_t offset);
extern void getQueryBufferObjectiv(uint32_t id, uint32_t buffer, QueryObjectParameterName pname, std::intptr_t offset);
extern void
  getQueryBufferObjectui64v(uint32_t id, uint32_t buffer, QueryObjectParameterName pname, std::intptr_t offset);
extern void getQueryBufferObjectuiv(uint32_t id, uint32_t buffer, QueryObjectParameterName pname, std::intptr_t offset);
extern void getTextureImage(
  uint32_t texture, int32_t level, PixelFormat format, PixelType type, core::SizeType bufSize, void* pixels);
extern void getTextureLevelParameter(uint32_t texture, int32_t level, GetTextureParameter pname, float* params);
extern void getTextureLevelParameter(uint32_t texture, int32_t level, GetTextureParameter pname, int32_t* params);
extern void getTextureParameterI(uint32_t texture, GetTextureParameter pname, int32_t* params);
extern void getTextureParameterI(uint32_t texture, GetTextureParameter pname, uint32_t* params);
extern void getTextureParameter(uint32_t texture, GetTextureParameter pname, float* params);
extern void getTextureParameter(uint32_t texture, GetTextureParameter pname, int32_t* params);
extern void getTextureSubImage(uint32_t texture,
                               int32_t level,
                               int32_t xoffset,
                               int32_t yoffset,
                               int32_t zoffset,
                               core::SizeType width,
                               core::SizeType height,
                               core::SizeType depth,
                               PixelFormat format,
                               PixelType type,
                               core::SizeType bufSize,
                               void* pixels);
extern void getTransformFeedbackI(uint32_t xfb, TransformFeedbackPName pname, uint32_t index, int64_t* param);
extern void getTransformFeedbackI(uint32_t xfb, TransformFeedbackPName pname, uint32_t index, int32_t* param);
extern void getTransformFeedback(uint32_t xfb, TransformFeedbackPName pname, int32_t* param);
extern void getVertexArrayIndexed64(uint32_t vaobj, uint32_t index, VertexArrayPName pname, int64_t* param);
extern void getVertexArrayIndexed(uint32_t vaobj, uint32_t index, VertexArrayPName pname, int32_t* param);
extern void getVertexArray(uint32_t vaobj, VertexArrayPName pname, int32_t* param);
extern void getnCompressedTexImage(TextureTarget target, int32_t lod, core::SizeType bufSize, void* pixels);
extern void getnTexImage(
  TextureTarget target, int32_t level, PixelFormat format, PixelType type, core::SizeType bufSize, void* pixels);
extern void getnUniform(uint32_t program, int32_t location, core::SizeType bufSize, double* params);
extern void getnUniform(uint32_t program, int32_t location, core::SizeType bufSize, float* params);
extern void getnUniform(uint32_t program, int32_t location, core::SizeType bufSize, int32_t* params);
extern void getnUniform(uint32_t program, int32_t location, core::SizeType bufSize, uint32_t* params);
extern void invalidateNamedFramebufferData(uint32_t framebuffer,
                                           core::SizeType numAttachments,
                                           const FramebufferAttachment* attachments);
extern void invalidateNamedFramebufferSubData(uint32_t framebuffer,
                                              core::SizeType numAttachments,
                                              const FramebufferAttachment* attachments,
                                              int32_t x,
                                              int32_t y,
                                              core::SizeType width,
                                              core::SizeType height);
extern void* mapNamedBuffer(uint32_t buffer, BufferAccess access);
extern void* mapNamedBufferRange(uint32_t buffer,
                                 std::intptr_t offset,
                                 std::size_t length,
                                 core::Bitfield<MapBufferAccessMask> access);
extern void memoryBarrierByRegion(core::Bitfield<MemoryBarrierMask> barriers);
extern void namedBufferData(uint32_t buffer, std::size_t size, const void* data, BufferUsage usage);
extern void
  namedBufferStorage(uint32_t buffer, std::size_t size, const void* data, core::Bitfield<BufferStorageMask> flags);
extern void namedBufferSubData(uint32_t buffer, std::intptr_t offset, std::size_t size, const void* data);
extern void namedFramebufferDrawBuffer(uint32_t framebuffer, ColorBuffer buf);
extern void namedFramebufferDrawBuffers(uint32_t framebuffer, core::SizeType n, const ColorBuffer* bufs);
extern void namedFramebufferParameter(uint32_t framebuffer, FramebufferParameterName pname, int32_t param);
extern void namedFramebufferReadBuffer(uint32_t framebuffer, ColorBuffer src);
extern void namedFramebufferRenderbuffer(uint32_t framebuffer,
                                         FramebufferAttachment attachment,
                                         RenderbufferTarget renderbuffertarget,
                                         uint32_t renderbuffer);
extern void
  namedFramebufferTexture(uint32_t framebuffer, FramebufferAttachment attachment, uint32_t texture, int32_t level);
extern void namedFramebufferTextureLayer(
  uint32_t framebuffer, FramebufferAttachment attachment, uint32_t texture, int32_t level, int32_t layer);
extern void namedRenderbufferStorage(uint32_t renderbuffer,
                                     InternalFormat internalformat,
                                     core::SizeType width,
                                     core::SizeType height);
extern void namedRenderbufferStorageMultisample(uint32_t renderbuffer,
                                                core::SizeType samples,
                                                InternalFormat internalformat,
                                                core::SizeType width,
                                                core::SizeType height);
extern void readnPixel(int32_t x,
                       int32_t y,
                       core::SizeType width,
                       core::SizeType height,
                       PixelFormat format,
                       PixelType type,
                       core::SizeType bufSize,
                       void* data);
extern void textureBarrier();
extern void textureBuffer(uint32_t texture, SizedInternalFormat internalformat, uint32_t buffer);
extern void textureBufferRange(
  uint32_t texture, SizedInternalFormat internalformat, uint32_t buffer, std::intptr_t offset, std::size_t size);
extern void textureParameterI(uint32_t texture, TextureParameterName pname, const int32_t* params);
extern void textureParameterI(uint32_t texture, TextureParameterName pname, const uint32_t* params);
extern void textureParameter(uint32_t texture, TextureParameterName pname, float param);
extern void textureParameter(uint32_t texture, TextureParameterName pname, const float* param);
extern void textureParameter(uint32_t texture, TextureParameterName pname, int32_t param);
extern void textureParameter(uint32_t texture, TextureParameterName pname, const int32_t* param);
extern void
  textureStorage1D(uint32_t texture, core::SizeType levels, SizedInternalFormat internalformat, core::SizeType width);
extern void textureStorage2D(uint32_t texture,
                             core::SizeType levels,
                             SizedInternalFormat internalformat,
                             core::SizeType width,
                             core::SizeType height);
extern void textureStorage2DMultisample(uint32_t texture,
                                        core::SizeType samples,
                                        SizedInternalFormat internalformat,
                                        core::SizeType width,
                                        core::SizeType height,
                                        bool fixedsamplelocations);
extern void textureStorage3D(uint32_t texture,
                             core::SizeType levels,
                             SizedInternalFormat internalformat,
                             core::SizeType width,
                             core::SizeType height,
                             core::SizeType depth);
extern void textureStorage3DMultisample(uint32_t texture,
                                        core::SizeType samples,
                                        SizedInternalFormat internalformat,
                                        core::SizeType width,
                                        core::SizeType height,
                                        core::SizeType depth,
                                        bool fixedsamplelocations);
extern void textureSubImage1D(uint32_t texture,
                              int32_t level,
                              int32_t xoffset,
                              core::SizeType width,
                              PixelFormat format,
                              PixelType type,
                              const void* pixels);
extern void textureSubImage2D(uint32_t texture,
                              int32_t level,
                              int32_t xoffset,
                              int32_t yoffset,
                              core::SizeType width,
                              core::SizeType height,
                              PixelFormat format,
                              PixelType type,
                              const void* pixels);
extern void textureSubImage3D(uint32_t texture,
                              int32_t level,
                              int32_t xoffset,
                              int32_t yoffset,
                              int32_t zoffset,
                              core::SizeType width,
                              core::SizeType height,
                              core::SizeType depth,
                              PixelFormat format,
                              PixelType type,
                              const void* pixels);
extern void transformFeedbackBufferBase(uint32_t xfb, uint32_t index, uint32_t buffer);
extern void
  transformFeedbackBufferRange(uint32_t xfb, uint32_t index, uint32_t buffer, std::intptr_t offset, std::size_t size);
extern bool unmapNamedBuffer(uint32_t buffer);
extern void vertexArrayAttribBinding(uint32_t vaobj, uint32_t attribindex, uint32_t bindingindex);
extern void vertexArrayAttribFormat(
  uint32_t vaobj, uint32_t attribindex, int32_t size, VertexAttribType type, bool normalized, uint32_t relativeoffset);
extern void vertexArrayAttribIFormat(
  uint32_t vaobj, uint32_t attribindex, int32_t size, VertexAttribIType type, uint32_t relativeoffset);
extern void vertexArrayAttribLFormat(
  uint32_t vaobj, uint32_t attribindex, int32_t size, VertexAttribLType type, uint32_t relativeoffset);
extern void vertexArrayBindingDivisor(uint32_t vaobj, uint32_t bindingindex, uint32_t divisor);
extern void vertexArrayElementBuffer(uint32_t vaobj, uint32_t buffer);
extern void vertexArrayVertexBuffer(
  uint32_t vaobj, uint32_t bindingindex, uint32_t buffer, std::intptr_t offset, core::SizeType stride);
extern void vertexArrayVertexBuffers(uint32_t vaobj,
                                     uint32_t first,
                                     core::SizeType count,
                                     const uint32_t* buffers,
                                     const std::intptr_t* offsets,
                                     const core::SizeType* strides);
} // namespace gl::api
