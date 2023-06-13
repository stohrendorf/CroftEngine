#pragma once

#include "api/soglb_core.hpp" // IWYU pragma: export

#include <glm/fwd.hpp>

namespace gl::api
{
enum class BufferTarget : core::EnumType;
enum class PixelFormat : core::EnumType;
enum class SizedInternalFormat : core::EnumType;
enum class ProgramInterface : core::EnumType;
enum class TextureTarget : core::EnumType;
enum class ShaderType : core::EnumType;
enum class ObjectIdentifier : core::EnumType;
} // namespace gl::api

namespace gl
{
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<api::ObjectIdentifier _Identifier>
class BindableResource;
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<typename T, api::BufferTarget _Target>
class Buffer;
class CImgWrapper;
class Font;
class TextureAttachment;
class Framebuffer;
class FrameBufferBuilder;
template<typename TStorage>
class Image;
template<typename T,
         // NOLINTNEXTLINE(bugprone-reserved-identifier)
         glm::length_t _Channels,
         // NOLINTNEXTLINE(bugprone-reserved-identifier)
         api::PixelFormat _PixelFormat,
         // NOLINTNEXTLINE(bugprone-reserved-identifier)
         api::SizedInternalFormat _SizedInternalFormat,
         // NOLINTNEXTLINE(bugprone-reserved-identifier)
         bool _Premultiplied,
         size_t Alignment>
struct Pixel;
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<api::ProgramInterface _Type>
class ProgramInterface;
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<api::ProgramInterface _Type>
class LocatableProgramInterface;
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<api::ProgramInterface _Type, api::BufferTarget _Target>
class ProgramBlock;
class Uniform;
class Program;
class RenderState;
class Sampler;
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<api::ShaderType _Type>
class Shader;
class Texture;
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<typename TTexture>
class TextureHandle;
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<api::TextureTarget _Target, typename PixelT>
class TextureImpl;
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<typename _PixelT>
class Texture2D;
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<typename _PixelT>
class Texture2DArray;
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<typename _T>
class TextureDepth;
template<typename IndexT, typename VertexT0, typename... VertexTs>
class VertexArray;
template<typename T>
class VertexAttribute;
template<typename T>
class VertexBuffer;
class Window;
template<typename T>
class ElementArrayBuffer;
class Font;
} // namespace gl
