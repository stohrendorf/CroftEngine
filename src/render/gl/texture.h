#pragma once

#include "bindableresource.h"

#include <boost/throw_exception.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <utility>
#include <vector>

namespace render::gl
{
class Texture : public BindableResource
{
protected:
  explicit Texture(Allocator allocator,
                   Binder binder,
                   Deleter deleter,
                   const ::gl::ObjectIdentifier identifier,
                   const std::string& label)
      : BindableResource{std::move(allocator), std::move(binder), std::move(deleter), identifier, label}
  {
  }
};

template<::gl::TextureTarget _Target, typename _PixelT>
class TextureImpl : public Texture
{
protected:
  explicit TextureImpl(const std::string& label = {})
      : Texture{
        [](::gl::core::SizeType n, ::gl::core::Handle* textures) { ::gl::createTextures(_Target, n, textures); },
        [](const uint32_t handle) { bindTexture(_Target, handle); },
        ::gl::deleteTextures,
        ::gl::ObjectIdentifier::Texture,
        label}
  {
  }

public:
  static constexpr auto Target = _Target;
  using Pixel = _PixelT;

  TextureImpl<_Target, _PixelT>& set(const ::gl::TextureMinFilter value)
  {
    GL_ASSERT(
      ::gl::textureParameter(getHandle(), ::gl::TextureParameterName::TextureMinFilter, static_cast<int32_t>(value)));
    return *this;
  }

  TextureImpl<_Target, _PixelT>& set(const ::gl::TextureMagFilter value)
  {
    GL_ASSERT(
      ::gl::textureParameter(getHandle(), ::gl::TextureParameterName::TextureMagFilter, static_cast<int32_t>(value)));
    return *this;
  }

  TextureImpl<_Target, _PixelT>& set(const ::gl::TextureCompareMode value)
  {
    GL_ASSERT(
      ::gl::textureParameter(getHandle(), ::gl::TextureParameterName::TextureCompareMode, static_cast<int32_t>(value)));
    return *this;
  }

  TextureImpl<_Target, _PixelT>& set(const ::gl::DepthFunction value)
  {
    GL_ASSERT(
      ::gl::textureParameter(getHandle(), ::gl::TextureParameterName::TextureCompareFunc, static_cast<int32_t>(value)));
    return *this;
  }

  TextureImpl<_Target, _PixelT>& set(const ::gl::TextureParameterName param, const ::gl::TextureWrapMode value)
  {
    GL_ASSERT(::gl::textureParameter(getHandle(), param, static_cast<int32_t>(value)));
    return *this;
  }

  TextureImpl<_Target, _PixelT>& setBorderColor(const glm::vec4& value)
  {
    GL_ASSERT(
      ::gl::textureParameter(getHandle(), ::gl::TextureParameterName::TextureBorderColor, glm::value_ptr(value)));
    return *this;
  }

  [[nodiscard]] static ::gl::CopyImageSubDataTarget getSubDataTarget()
  {
#define SOGLB_CONVERT_TYPE(x) \
  case ::gl::TextureTarget::x: return ::gl::CopyImageSubDataTarget::x
    switch(_Target)
    {
      SOGLB_CONVERT_TYPE(Texture1d);
      SOGLB_CONVERT_TYPE(Texture2d);
      SOGLB_CONVERT_TYPE(Texture3d);
      SOGLB_CONVERT_TYPE(TextureCubeMap);
      SOGLB_CONVERT_TYPE(Texture1dArray);
      SOGLB_CONVERT_TYPE(Texture2dArray);
      SOGLB_CONVERT_TYPE(TextureRectangle);
      SOGLB_CONVERT_TYPE(Texture2dMultisample);
      SOGLB_CONVERT_TYPE(Texture2dMultisampleArray);
      SOGLB_CONVERT_TYPE(TextureCubeMapArray);
    default: BOOST_THROW_EXCEPTION(std::domain_error("Texture type not suitable for copy sub-data operation"));
    }
#undef SOGLB_CONVERT_TYPE
  }
};
} // namespace render::gl
