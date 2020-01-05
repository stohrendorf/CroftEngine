#pragma once

#include "texture.h"

namespace render::gl
{
template<typename _PixelT>
class Texture2DArray : public TextureImpl<::gl::TextureTarget::Texture2dArray, _PixelT>
{
public:
  explicit Texture2DArray(const std::string& label = {})
      : TextureImpl<::gl::TextureTarget::Texture2dArray, _PixelT>{label}
  {
  }

  Texture2DArray<_PixelT>& allocate(const glm::ivec3& size, int levels = 1)
  {
    BOOST_ASSERT(levels > 0);
    BOOST_ASSERT(size.x > 0);
    BOOST_ASSERT(size.y > 0);
    BOOST_ASSERT(size.z > 0);

    if(m_levels != levels || m_size != size)
    {
      bind();
      GL_ASSERT(::gl::texStorage3D(Target, levels, Pixel::InternalFormat, size.x, size.y, size.z));
      m_size = size;
      m_levels = levels;
    }

    return *this;
  }

  Texture2DArray<_PixelT>& assign(const gsl::not_null<const _PixelT*>& data, int z, int level = 0)
  {
    BOOST_ASSERT(m_levels > 0);
    BOOST_ASSERT(level >= 0 && level < m_levels);
    BOOST_ASSERT(z >= 0 && z < m_size.z);

    const int levelDiv = 1 << level;

    bind();
    GL_ASSERT(::gl::texSubImage3D(Target,
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
  int m_levels = -1;
};
} // namespace render::gl
