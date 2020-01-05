#pragma once

#include "bindableresource.h"

#include <boost/throw_exception.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

namespace render::gl
{
class Texture : public BindableResource
{
protected:
  explicit Texture(::gl::TextureTarget target, const std::string& label = {})
      : BindableResource{::gl::genTextures,
                         [target](const uint32_t handle) { bindTexture(target, handle); },
                         ::gl::deleteTextures,
                         ::gl::ObjectIdentifier::Texture,
                         label}
  {
  }
};

template<::gl::TextureTarget _Target, typename _PixelT>
class TextureImpl : public Texture
{
protected:
  explicit TextureImpl(const std::string& label = {})
      : Texture{_Target, label}
  {
  }

public:
  static constexpr auto Target = _Target;
  using Pixel = _PixelT;

  TextureImpl<_Target, _PixelT>& set(const ::gl::TextureMinFilter value)
  {
    bind();
    GL_ASSERT(::gl::texParameter(_Target, ::gl::TextureParameterName::TextureMinFilter, static_cast<int32_t>(value)));
    return *this;
  }

  TextureImpl<_Target, _PixelT>& set(const ::gl::TextureMagFilter value)
  {
    bind();
    GL_ASSERT(::gl::texParameter(_Target, ::gl::TextureParameterName::TextureMagFilter, static_cast<int32_t>(value)));
    return *this;
  }

  TextureImpl<_Target, _PixelT>& set(const ::gl::TextureCompareMode value)
  {
    bind();
    GL_ASSERT(::gl::texParameter(_Target, ::gl::TextureParameterName::TextureCompareMode, static_cast<int32_t>(value)));
    return *this;
  }

  TextureImpl<_Target, _PixelT>& set(const ::gl::DepthFunction value)
  {
    bind();
    GL_ASSERT(::gl::texParameter(_Target, ::gl::TextureParameterName::TextureCompareFunc, static_cast<int32_t>(value)));
    return *this;
  }

  TextureImpl<_Target, _PixelT>& set(const ::gl::TextureParameterName param, const ::gl::TextureWrapMode value)
  {
    bind();
    GL_ASSERT(::gl::texParameter(_Target, param, static_cast<int32_t>(value)));
    return *this;
  }

  TextureImpl<_Target, _PixelT>& setBorderColor(const glm::vec4& value)
  {
    bind();
    GL_ASSERT(::gl::texParameter(_Target, ::gl::TextureParameterName::TextureBorderColor, glm::value_ptr(value)));
    return *this;
  }

  [[nodiscard]] ::gl::CopyImageSubDataTarget getSubDataTarget() const
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
