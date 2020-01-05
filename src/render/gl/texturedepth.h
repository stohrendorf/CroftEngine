#pragma once

#include "texture.h"

namespace render::gl
{
template<typename _T>
class TextureDepth : public TextureImpl<::gl::TextureTarget::Texture2d, ScalarDepth<_T>>
{
public:
  explicit TextureDepth(const std::string& label = {})
      : TextureImpl<::gl::TextureTarget::Texture2d, Pixel>{label}
  {
  }

  TextureDepth<_T>& allocate(const glm::ivec2& size)
  {
    BOOST_ASSERT(size.x > 0);
    BOOST_ASSERT(size.y > 0);

    if(m_size != size)
    {
      bind();
      GL_ASSERT(::gl::texStorage2D(Target, 1, Pixel::InternalFormat, size.x, size.y));
      m_size = size;
    }

    return *this;
  }

  TextureDepth<_T>& allocateMutable(const glm::ivec2& size)
  {
    BOOST_ASSERT(size.x > 0);
    BOOST_ASSERT(size.y > 0);

    if(m_size != size)
    {
      bind();
      GL_ASSERT(::gl::texImage2D(
        Target, 0, Pixel::InternalFormat, size.x, size.y, 0, Pixel::PixelFormat, Pixel::PixelType, nullptr));
      m_size = size;
    }

    return *this;
  }

  TextureDepth<_T>& assign(const ScalarDepth<_T>* data)
  {
    bind();
    GL_ASSERT(::gl::texSubImage2D(Target, 0, 0, 0, m_size.x, m_size.y, Pixel::PixelFormat, Pixel::PixelType, data));
    return *this;
  }

  TextureDepth<_T>& fill(const ScalarDepth<_T>& value)
  {
    Expects(m_size.x >= 0 && m_size.y >= 0);
    std::vector<Pixel> data(m_size.x * m_size.y, value);
    return assign(data.data());
  }

  TextureDepth<_T>& fill(const _T& value)
  {
    return fill(Pixel{value});
  }

  void copyFrom(const TextureDepth<_T>& src)
  {
    allocateMutable(src.m_size);

    GL_ASSERT(::gl::copyImageSubData(src.getHandle(),
                                     src.getSubDataTarget(),
                                     0,
                                     0,
                                     0,
                                     0,
                                     getHandle(),
                                     getSubDataTarget(),
                                     0,
                                     0,
                                     0,
                                     0,
                                     src.m_size.x,
                                     src.m_size.y,
                                     1));
  }

private:
  glm::ivec2 m_size{-1};
};
} // namespace render::gl
