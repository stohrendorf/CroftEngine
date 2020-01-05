#pragma once

#include "texture.h"

namespace render::gl
{
template<typename _PixelT>
class Texture2D : public TextureImpl<::gl::TextureTarget::Texture2d, _PixelT>
{
public:
  explicit Texture2D(const std::string& label = {})
      : TextureImpl<::gl::TextureTarget::Texture2d, _PixelT>{label}
  {
  }

  Texture2D<_PixelT>& allocate(const glm::ivec2& size, int levels = 1)
  {
    BOOST_ASSERT(levels > 0);
    BOOST_ASSERT(size.x > 0);
    BOOST_ASSERT(size.y > 0);

    if(m_levels != levels || m_size != size)
    {
      bind();
      GL_ASSERT(::gl::texStorage2D(Target, levels, Pixel::InternalFormat, size.x, size.y));
      m_size = size;
      m_levels = levels;
    }

    return *this;
  }

  Texture2D<_PixelT>& allocateMutable(glm::ivec2 size, int levels = 1)
  {
    BOOST_ASSERT(levels > 0);
    BOOST_ASSERT(size.x > 0);
    BOOST_ASSERT(size.y > 0);

    if(m_levels != levels || m_size != size)
    {
      bind();

      m_size = size;
      m_levels = levels;

      for(int lvl = 0; lvl < levels; ++lvl)
      {
        GL_ASSERT(::gl::texImage2D(Target,
                                   lvl,
                                   Pixel::InternalFormat,
                                   glm::max(1, size.x),
                                   glm::max(1, size.y),
                                   0,
                                   Pixel::PixelFormat,
                                   Pixel::PixelType,
                                   nullptr));
        size /= 2;
      }
    }

    return *this;
  }

  Texture2D<_PixelT>& assign(const gsl::not_null<const _PixelT*>& data, int level = 0)
  {
    BOOST_ASSERT(m_levels > 0);
    BOOST_ASSERT(level < m_levels);

    const int levelDiv = 1 << level;

    bind();
    GL_ASSERT(::gl::texSubImage2D(Target,
                                  level,
                                  0,
                                  0,
                                  glm::max(1, m_size.x / levelDiv),
                                  glm::max(1, m_size.y / levelDiv),
                                  Pixel::PixelFormat,
                                  Pixel::PixelType,
                                  data.get()));
    return *this;
  }

private:
  glm::ivec2 m_size{-1};
  int m_levels = -1;
};
} // namespace render::gl
