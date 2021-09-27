#pragma once

#include "bindableresource.h" // IWYU pragma: export

#include <glm/gtc/type_ptr.hpp>
#include <string_view>
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
                   const std::string_view& label)
      : BindableResource{std::move(allocator), std::move(binder), std::move(deleter), identifier, label}
  {
  }
};

// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<api::TextureTarget _Target, typename _PixelT>
class TextureImpl : public Texture
{
protected:
  explicit TextureImpl(const std::string_view& label)
      : Texture{[](const api::core::SizeType n, api::core::Handle* textures) { createTextures(_Target, n, textures); },
                [](const uint32_t handle) { bindTexture(_Target, handle); },
                api::deleteTextures,
                api::ObjectIdentifier::Texture,
                label}
  {
  }

public:
  static constexpr auto Target = _Target;
  using Pixel = _PixelT;

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
    else static_assert(!std::is_same_v<_PixelT, _PixelT>, "Texture type not suitable for copy sub-data operation");
#undef SOGLB_CONVERT_TYPE
  }

  TextureImpl<_Target, _PixelT>& clear(const _PixelT& pixel, int level = 0)
  {
    GL_ASSERT(api::clearTexImage(getHandle(), level, Pixel::PixelFormat, Pixel::PixelType, &pixel));
    return *this;
  }
};
} // namespace gl
