#pragma once

#include "texture.h"

namespace render::gl
{
template<typename _T>
class TextureDepth : public TextureImpl<::gl::TextureTarget::Texture2d, ScalarDepth<_T>>
{
public:
  explicit TextureDepth(const glm::ivec2& size, const std::string& label = {})
      : TextureImpl<::gl::TextureTarget::Texture2d, Pixel>{label}
      , m_size{size}
  {
    BOOST_ASSERT(size.x > 0);
    BOOST_ASSERT(size.y > 0);

    GL_ASSERT(::gl::textureStorage2D(getHandle(), 1, Pixel::InternalFormat, size.x, size.y));
  }

  TextureDepth<_T>& assign(const ScalarDepth<_T>* data)
  {
    GL_ASSERT(
      ::gl::textureSubImage2D(getHandle(), 0, 0, 0, m_size.x, m_size.y, Pixel::PixelFormat, Pixel::PixelType, data));
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
