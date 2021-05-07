#pragma once

#include "texture.h"

namespace gl
{
// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<typename _PixelT>
class Texture2D final : public TextureImpl<api::TextureTarget::Texture2d, _PixelT>
{
public:
  using typename TextureImpl<api::TextureTarget::Texture2d, _PixelT>::Pixel;
  using TextureImpl<api::TextureTarget::Texture2d, _PixelT>::getHandle;

  explicit Texture2D(const glm::ivec2& size, const std::string& label = {})
      : Texture2D<_PixelT>{size, 1, label}
  {
  }

  explicit Texture2D(const glm::ivec2& size, int levels, const std::string& label = {})
      : TextureImpl<api::TextureTarget::Texture2d, _PixelT>{label}
      , m_size{size}
  {
    BOOST_ASSERT(levels > 0);
    BOOST_ASSERT(size.x > 0);
    BOOST_ASSERT(size.y > 0);
    GL_ASSERT(api::textureStorage2D(getHandle(), levels, Pixel::SizedInternalFormat, size.x, size.y));
  }

  Texture2D<_PixelT>& assign(const gsl::not_null<const _PixelT*>& data, int level = 0)
  {
    const int levelDiv = 1 << level;
    const auto sizeX = glm::max(1, m_size.x / levelDiv);
    const auto sizeY = glm::max(1, m_size.y / levelDiv);

    GL_ASSERT(
      api::textureSubImage2D(getHandle(), level, 0, 0, sizeX, sizeY, Pixel::PixelFormat, Pixel::PixelType, data.get()));
    return *this;
  }

  Texture2D<_PixelT>& fill(const _PixelT& pixel, int level = 0)
  {
    const int levelDiv = 1 << level;
    const auto sizeX = glm::max(1, m_size.x / levelDiv);
    const auto sizeY = glm::max(1, m_size.y / levelDiv);
    std::vector<_PixelT> pixels;
    pixels.resize(sizeX * sizeY, pixel);

    GL_ASSERT(api::textureSubImage2D(
      getHandle(), level, 0, 0, sizeX, sizeY, Pixel::PixelFormat, Pixel::PixelType, pixels.data()));
    return *this;
  }

  [[nodiscard]] const glm::ivec2& size() const noexcept
  {
    return m_size;
  }

private:
  const glm::ivec2 m_size;
};
} // namespace gl
