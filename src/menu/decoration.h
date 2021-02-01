#pragma once

#include "loader/file/color.h"
#include "ui/label.h"

#include <gl/image.h>

namespace menu
{
class Decoration
{
public:
  virtual ~Decoration() = default;

  virtual void draw(gl::Image<gl::SRGBA8>& img, const glm::ivec2& xy) const = 0;
};

class SpriteDecoration final : public Decoration
{
public:
  SpriteDecoration(const glm::ivec2& offset, size_t glyph, const ui::CachedFont& cachedFont)
      : m_offset{offset}
      , m_glyph{glyph}
      , m_cachedFont{cachedFont}
  {
  }

  void draw(gl::Image<gl::SRGBA8>& img, const glm::ivec2& xy) const override
  {
    m_cachedFont.draw(m_glyph, xy + m_offset, img);
  }

private:
  const glm::ivec2 m_offset;
  const size_t m_glyph;
  const ui::CachedFont& m_cachedFont;
};

class LineDecoration final : public Decoration
{
public:
  LineDecoration(const glm::ivec2& offset, const glm::ivec2& size, const gl::SRGBA8& color)
      : m_offset{offset}
      , m_size{size}
      , m_color{color}
  {
  }

  void draw(gl::Image<gl::SRGBA8>& img, const glm::ivec2& xy) const override
  {
    img.line(xy + m_offset, xy + m_offset + m_size, m_color);
  }

private:
  const glm::ivec2 m_offset;
  const glm::ivec2 m_size;
  const gl::SRGBA8 m_color;
};

class BoxDecoration final : public Decoration
{
public:
  BoxDecoration(const glm::ivec2& offset, const glm::ivec2& size, const loader::file::Palette& palette)
      : m_offset{offset}
      , m_size{size}
      , m_color1{palette.colors[15].toTextureColor()}
      , m_color2{palette.colors[31].toTextureColor()}
  {
  }

  void draw(gl::Image<gl::SRGBA8>& img, const glm::ivec2& xy) const override
  {
    const auto xy0 = xy + m_offset;
    const auto xy1 = xy0 + m_size;
    img.lineDxy(xy0, xy1 + glm::ivec2{2, 0}, m_color1);
    img.lineDxy(xy0 - glm::ivec2{0, 1}, {m_size.x + 2, 0}, m_color1);
    img.lineDxy(xy0, {m_size.x + 1, 0}, m_color2);
    img.lineDxy({xy1.x, xy0.y}, {0, m_size.y + 1}, m_color1);
    img.lineDxy({xy1.x + 1, xy0.y}, {0, m_size.y + 2}, m_color2);
    img.lineDxy(xy1, {-m_size.x - 1, 0}, m_color1);
    img.lineDxy(xy1 + glm::ivec2{0, 1}, {-m_size.x - 2, 0}, m_color2);
    img.lineDxy({xy0.x - 1, xy1.y}, {0, -m_size.y - 2}, m_color1);
    img.lineDxy({xy0.x, xy1.y}, {0, -m_size.y - 1}, m_color2);
  }

private:
  const glm::ivec2 m_offset;
  const glm::ivec2 m_size;
  const gl::SRGBA8 m_color1;
  const gl::SRGBA8 m_color2;
};
} // namespace menu
