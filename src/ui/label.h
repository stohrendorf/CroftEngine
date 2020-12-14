#pragma once

#include "loader/file/color.h"
#include "loader/file/datatypes.h"

#include <cstdint>
#include <gl/cimgwrapper.h>
#include <string>

namespace loader::file
{
struct Palette;
} // namespace loader::file

namespace ui
{
constexpr int FontBaseScale = 0x10000;

class CachedFont
{
  struct Glyph
  {
    explicit Glyph(gl::CImgWrapper&& image, int shiftX, int shiftY)
        : image{std::move(image)}
        , shiftX{shiftX}
        , shiftY{shiftY}
    {
    }

    gl::CImgWrapper image;
    const int shiftX;
    const int shiftY;
  };

  mutable std::vector<Glyph> m_glyphs;
  const int m_scale;

  static gl::CImgWrapper extractChar(const loader::file::Sprite& sprite, const int scale)
  {
    BOOST_ASSERT(sprite.image != nullptr);

    const auto dstW = glm::abs(sprite.render1.x - sprite.render0.x) * scale / FontBaseScale;
    const auto dstH = glm::abs(sprite.render1.y - sprite.render0.y) * scale / FontBaseScale;

    gl::CImgWrapper src{*sprite.image};
    src.crop(sprite.uv0.toNearestPx(sprite.image->width()).x,
             sprite.uv0.toNearestPx(sprite.image->height()).y,
             sprite.uv1.toNearestPx(sprite.image->width()).x,
             sprite.uv1.toNearestPx(sprite.image->height()).y);
    src.resize(dstW, dstH);

    return src;
  }

public:
  explicit CachedFont(const loader::file::SpriteSequence& sequence, const int scale = FontBaseScale)
      : m_scale{scale}
  {
    std::transform(sequence.sprites.begin(),
                   sequence.sprites.end(),
                   std::back_inserter(m_glyphs),
                   [scale](const loader::file::Sprite& spr) {
                     return Glyph{extractChar(spr, scale), spr.render0.x, spr.render0.y};
                   });
  }

  [[nodiscard]] const Glyph& get(size_t n) const
  {
    return m_glyphs.at(n);
  }

  void draw(size_t n, int x, int y, gl::Image<gl::SRGBA8>& img) const
  {
    auto& src = m_glyphs.at(n);
    x += src.shiftX;
    y += src.shiftY;

    for(int dy = 0; dy < src.image.height(); ++dy)
    {
      for(int dx = 0; dx < src.image.width(); ++dx)
      {
        img.set(x + dx, y + dy, src.image(dx, dy), true);
      }
    }
  }

  [[nodiscard]] int getScale() const noexcept
  {
    return m_scale;
  }
};

struct Label
{
  enum class Alignment
  {
    None,
    Top = None,
    Left = None,
    Center,
    Right,
    Bottom = Right
  };

  bool blink = false;
  Alignment alignX = Alignment::Left;
  Alignment alignY = Alignment::Top;
  bool fillBackground = false;
  bool outline = false;
  int16_t posX = 0;
  int16_t posY = 0;
  int16_t letterSpacing = 1;
  int16_t wordSpacing = 6;
  int16_t blinkTime = 0;
  mutable int16_t timeout = 0;
  int16_t bgndSizeX = 0;
  int16_t bgndSizeY = 0;
  int16_t bgndOffX = 0;
  int16_t bgndOffY = 0;
  int scale = FontBaseScale;
  std::string text;

  explicit Label(int16_t xpos, int16_t ypos, const std::string& string)
      : posX{xpos}
      , posY{ypos}
      , text{string, 0, std::min(std::string::size_type(64), string.size())}
  {
  }

  void draw(const CachedFont& font, gl::Image<gl::SRGBA8>& img, const loader::file::Palette& palette) const;

  int calcWidth() const;

  void addBackground(int16_t xsize, int16_t ysize, int16_t xoff, int16_t yoff)
  {
    bgndSizeX = xsize;
    bgndSizeY = ysize;
    bgndOffX = xoff;
    bgndOffY = yoff;
    fillBackground = true;
  }

  void removeBackground()
  {
    fillBackground = false;
  }

  void flashText(bool blink, int16_t blinkTime)
  {
    this->blink = blink;
    if(blink)
    {
      this->blinkTime = blinkTime;
      timeout = blinkTime;
    }
  }
};
} // namespace ui
