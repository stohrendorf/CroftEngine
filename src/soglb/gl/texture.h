#pragma once

#include "bindableresource.h"

#include <glm/gtc/type_ptr.hpp>
#include <utility>

namespace gl
{
class Texture : public BindableResource
{
protected:
  explicit Texture(Allocator allocator,
                   Binder binder,
                   Deleter deleter,
                   const api::ObjectIdentifier identifier,
                   const std::string& label)
      : BindableResource{std::move(allocator), std::move(binder), std::move(deleter), identifier, label}
  {
  }
};

template<api::TextureTarget _Target, typename PixelT>
class TextureImpl : public Texture
{
protected:
  explicit TextureImpl(const std::string& label = {})
      : Texture{[](const api::core::SizeType n, api::core::Handle* textures) { createTextures(_Target, n, textures); },
                [](const uint32_t handle) { bindTexture(_Target, handle); },
                api::deleteTextures,
                api::ObjectIdentifier::Texture,
                label}
  {
  }

public:
  static constexpr auto Target = _Target;
  using Pixel = PixelT;

  TextureImpl<_Target, PixelT>& set(const api::TextureMinFilter value)
  {
    GL_ASSERT(
      api::textureParameter(getHandle(), api::TextureParameterName::TextureMinFilter, static_cast<int32_t>(value)));
    return *this;
  }

  TextureImpl<_Target, PixelT>& set(const api::TextureMagFilter value)
  {
    GL_ASSERT(
      api::textureParameter(getHandle(), api::TextureParameterName::TextureMagFilter, static_cast<int32_t>(value)));
    return *this;
  }

  TextureImpl<_Target, PixelT>& set(const api::TextureCompareMode value)
  {
    GL_ASSERT(
      api::textureParameter(getHandle(), api::TextureParameterName::TextureCompareMode, static_cast<int32_t>(value)));
    return *this;
  }

  TextureImpl<_Target, PixelT>& set(const api::DepthFunction value)
  {
    GL_ASSERT(
      api::textureParameter(getHandle(), api::TextureParameterName::TextureCompareFunc, static_cast<int32_t>(value)));
    return *this;
  }

  TextureImpl<_Target, PixelT>& set(const api::TextureParameterName param, const api::TextureWrapMode value)
  {
    GL_ASSERT(api::textureParameter(getHandle(), param, static_cast<int32_t>(value)));
    return *this;
  }

  TextureImpl<_Target, PixelT>& setBorderColor(const glm::vec4& value)
  {
    GL_ASSERT(api::textureParameter(getHandle(), api::TextureParameterName::TextureBorderColor, glm::value_ptr(value)));
    return *this;
  }

  [[nodiscard]] auto getWidth() const
  {
    int32_t w;
    GL_ASSERT(api::getTextureParameter(getHandle(), api::GetTextureParameter::TextureWidth, &w));
    return w;
  }

  [[nodiscard]] auto getHeight() const
  {
    int32_t h;
    GL_ASSERT(api::getTextureParameter(getHandle(), api::GetTextureParameter::TextureHeight, &h));
    return h;
  }

  [[nodiscard]] static api::CopyImageSubDataTarget getSubDataTarget()
  {
#define SOGLB_CONVERT_TYPE(x)                    \
  if constexpr(api::TextureTarget::x == _Target) \
  return api::CopyImageSubDataTarget::x
    SOGLB_CONVERT_TYPE(Texture1d);
    else SOGLB_CONVERT_TYPE(Texture2d);
    else SOGLB_CONVERT_TYPE(Texture3d);
    else SOGLB_CONVERT_TYPE(TextureCubeMap);
    else SOGLB_CONVERT_TYPE(Texture1dArray);
    else SOGLB_CONVERT_TYPE(Texture2dArray);
    else SOGLB_CONVERT_TYPE(TextureRectangle);
    else SOGLB_CONVERT_TYPE(Texture2dMultisample);
    else SOGLB_CONVERT_TYPE(Texture2dMultisampleArray);
    else SOGLB_CONVERT_TYPE(TextureCubeMapArray);
    else static_assert(!std::is_same_v<PixelT, PixelT>, "Texture type not suitable for copy sub-data operation");
#undef SOGLB_CONVERT_TYPE
  }
};
} // namespace gl
