#pragma once

#include "bindableresource.h" // IWYU pragma: export

#include <boost/container/flat_map.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string_view>
#include <utility>

namespace gl
{
class Texture : public Resource<api::ObjectIdentifier::Texture>
{
protected:
  explicit Texture(Allocator allocator, Deleter deleter, const std::string_view& label)
      : Resource{std::move(allocator), std::move(deleter), label}
  {
  }

public:
  void generateMipmaps()
  {
    GL_ASSERT(gl::api::generateTextureMipmap(getHandle()));
  }
};

// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<api::TextureTarget _Target, typename _PixelT>
class TextureImpl : public Texture
{
  template<typename>
  friend class TextureHandle;

protected:
  explicit TextureImpl(const std::string_view& label)
      : Texture{[](const api::core::SizeType n, api::core::Handle* textures)
                {
                  createTextures(_Target, n, textures);
                },
                api::deleteTextures,
                label}
  {
    gsl_Assert(gl::api::isTexture(getHandle()));
  }

public:
  static constexpr auto Target = _Target;
  using Pixel = _PixelT;

  TextureImpl<_Target, _PixelT>& clear(const _PixelT& pixel, int level = 0)
  {
    GL_ASSERT(api::clearTexImage(getHandle(), level, Pixel::PixelFormat, Pixel::PixelType, &pixel));
    return *this;
  }

private:
  boost::container::flat_map<uint64_t, uint32_t> m_textureHandleReferences;
};
} // namespace gl
