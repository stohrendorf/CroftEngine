#pragma once

#include "texture.h"

#include <string_view>

namespace gl
{
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<typename _PixelT>
class Texture2DArray final : public TextureImpl<api::TextureTarget::Texture2dArray, _PixelT>
{
public:
  using typename TextureImpl<api::TextureTarget::Texture2dArray, _PixelT>::Pixel;
  using TextureImpl<api::TextureTarget::Texture2dArray, _PixelT>::getHandle;

  explicit Texture2DArray(const glm::ivec3& size, const std::string_view& label, int levels = 1)
      : TextureImpl<api::TextureTarget::Texture2dArray, _PixelT>{label}
      , m_size{size}
  {
    BOOST_ASSERT(levels > 0);
    BOOST_ASSERT(size.x > 0);
    BOOST_ASSERT(size.y > 0);
    BOOST_ASSERT(size.z > 0);

    GL_ASSERT(api::textureStorage3D(getHandle(), levels, Pixel::SizedInternalFormat, size.x, size.y, size.z));
  }

  Texture2DArray<_PixelT>& assign(const gsl::span<const _PixelT>& data, int z, int level = 0)
  {
    BOOST_ASSERT(z >= 0 && z < m_size.z);

    const int levelDiv = 1 << level;
    const auto size = glm::max(glm::ivec3{1, 1, 1}, m_size / levelDiv);
    gsl_Assert(gsl::narrow_cast<size_t>(size.x) * gsl::narrow_cast<size_t>(size.y) == data.size());

    GL_ASSERT(api::textureSubImage3D(
      getHandle(), level, 0, 0, z, size.x, size.y, 1, Pixel::PixelFormat, Pixel::PixelType, data.data()));
    return *this;
  }

private:
  glm::ivec3 m_size{-1};
};
} // namespace gl
