#pragma once

#include <boost/assert.hpp>
#include <gl/glassert.h>
#include <glm/vec2.hpp>
#include <string_view>

namespace gl
{
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<typename _T>
class TextureDepth final : public TextureImpl<api::TextureTarget::Texture2d, ScalarDepth<_T>>
{
public:
  using typename TextureImpl<api::TextureTarget::Texture2d, ScalarDepth<_T>>::Pixel;
  using TextureImpl<api::TextureTarget::Texture2d, ScalarDepth<_T>>::getHandle;

  explicit TextureDepth(const glm::ivec2& size, const std::string_view& label)
      : TextureImpl<api::TextureTarget::Texture2d, ScalarDepth<_T>>{label}
      , m_size{size}
  {
    BOOST_ASSERT(size.x > 0);
    BOOST_ASSERT(size.y > 0);

    GL_ASSERT(api::textureStorage2D(getHandle(), 1, Pixel::InternalFormat, size.x, size.y));
  }

  [[nodiscard]] const glm::ivec2& size() const
  {
    return m_size;
  }

private:
  glm::ivec2 m_size;
};
} // namespace gl
