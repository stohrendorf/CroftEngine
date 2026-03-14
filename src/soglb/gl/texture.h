#pragma once

#include "bindableresource.h" // IWYU pragma: export

#include <boost/container/flat_map.hpp>
#include <cstdint>
#include <gl/glassert.h>
#include <gl/resource.h>
#include <gsl-lite/gsl-lite.hpp>
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
  // ReSharper disable once CppMemberFunctionMayBeConst
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

  TextureImpl& clear(const _PixelT& pixel, const int level = 0)
  {
    GL_ASSERT(api::clearTexImage(getHandle(), level, Pixel::PixelFormat, Pixel::PixelType, &pixel));
    return *this;
  }

private:
  boost::container::flat_map<uint64_t, uint32_t> m_textureHandleReferences;
};
} // namespace gl