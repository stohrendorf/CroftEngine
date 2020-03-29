#pragma once

#include "texture.h"

namespace gl
{
template<typename _PixelT>
class Texture2DArray : public TextureImpl<api::TextureTarget::Texture2dArray, _PixelT>
{
public:
  using typename TextureImpl<api::TextureTarget::Texture2dArray, _PixelT>::Pixel;
  using TextureImpl<api::TextureTarget::Texture2dArray, _PixelT>::getHandle;

  explicit Texture2DArray(const glm::ivec3& size, int levels = 1, const std::string& label = {})
      : TextureImpl<api::TextureTarget::Texture2dArray, _PixelT>{label}
      , m_size{size}
  {
    BOOST_ASSERT(levels > 0);
    BOOST_ASSERT(size.x > 0);
    BOOST_ASSERT(size.y > 0);
    BOOST_ASSERT(size.z > 0);

    GL_ASSERT(api::textureStorage3D(getHandle(), levels, Pixel::InternalFormat, size.x, size.y, size.z));
  }

  Texture2DArray<_PixelT>& assign(const gsl::not_null<const _PixelT*>& data, int z, int level = 0)
  {
    BOOST_ASSERT(z >= 0 && z < m_size.z);

    const int levelDiv = 1 << level;

    GL_ASSERT(api::textureSubImage3D(getHandle(),
                                     level,
                                     0,
                                     0,
                                     z,
                                     glm::max(1, m_size.x / levelDiv),
                                     glm::max(1, m_size.y / levelDiv),
                                     1,
                                     Pixel::PixelFormat,
                                     Pixel::PixelType,
                                     data.get()));
    return *this;
  }

private:
  glm::ivec3 m_size{-1};
};
} // namespace gl
