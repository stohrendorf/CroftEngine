#pragma once

#include "texture.h"

namespace gl
{
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<typename _T>
class TextureDepth final : public TextureImpl<api::TextureTarget::Texture2d, ScalarDepth<_T>>
{
public:
  using typename TextureImpl<api::TextureTarget::Texture2d, ScalarDepth<_T>>::Pixel;
  using TextureImpl<api::TextureTarget::Texture2d, ScalarDepth<_T>>::getHandle;
  using TextureImpl<api::TextureTarget::Texture2d, ScalarDepth<_T>>::getSubDataTarget;

  explicit TextureDepth(const glm::ivec2& size, const std::string& label = {})
      : TextureImpl<api::TextureTarget::Texture2d, ScalarDepth<_T>>{label}
      , m_size{size}
  {
    BOOST_ASSERT(size.x > 0);
    BOOST_ASSERT(size.y > 0);

    GL_ASSERT(api::textureStorage2D(getHandle(), 1, Pixel::InternalFormat, size.x, size.y));
  }

  TextureDepth<_T>& assign(const ScalarDepth<_T>* data)
  {
    GL_ASSERT(
      api::textureSubImage2D(getHandle(), 0, 0, 0, m_size.x, m_size.y, Pixel::PixelFormat, Pixel::PixelType, data));
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
    GL_ASSERT(api::copyImageSubData(src.getHandle(),
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

  [[nodiscard]] const glm::ivec2& size() const
  {
    return m_size;
  }

private:
  const glm::ivec2 m_size;
};
} // namespace gl
