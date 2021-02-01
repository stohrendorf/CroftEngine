#pragma once

#include "loader/file/color.h"
#include "loader/file/datatypes.h"
#include "util.h"

#include <cstdint>
#include <gl/cimgwrapper.h>
#include <string>

namespace loader::file
{
struct Palette;
} // namespace loader::file

namespace ui
{
extern std::string makeAmmoString(const std::string& str);

constexpr int FontBaseScale = 0x10000;

class CachedFont
{
  struct Glyph
  {
    explicit Glyph(gl::CImgWrapper&& image, glm::ivec2 shift)
        : image{std::move(image)}
        , shift{shift}
    {
    }

    gl::CImgWrapper image;
    const glm::ivec2 shift;
  };

  mutable std::vector<Glyph> m_glyphs;
  const int m_scale;

  static gl::CImgWrapper extractChar(const loader::file::Sprite& sprite, const int scale)
  {
    BOOST_ASSERT(sprite.image != nullptr);

    const auto dstW = glm::abs(sprite.render1.x - sprite.render0.x) * scale / FontBaseScale;
    const auto dstH = glm::abs(sprite.render1.y - sprite.render0.y) * scale / FontBaseScale;

    gl::CImgWrapper src{*sprite.image};
    src.crop(sprite.uv0.toPx(sprite.image->width()).x,
             sprite.uv0.toPx(sprite.image->height()).y,
             sprite.uv1.toPx(sprite.image->width()).x,
             sprite.uv1.toPx(sprite.image->height()).y);
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
                     return Glyph{extractChar(spr, scale), spr.render0};
                   });
  }

  [[nodiscard]] const Glyph& get(size_t n) const
  {
    return m_glyphs.at(n);
  }

  void draw(size_t n, glm::ivec2 xy, gl::Image<gl::SRGBA8>& img) const
  {
    auto& src = m_glyphs.at(n);
    xy += src.shift;

    for(int dy = 0; dy < src.image.height(); ++dy)
    {
      for(int dx = 0; dx < src.image.width(); ++dx)
      {
        img.set(xy + glm::ivec2{dx, dy}, src.image(dx, dy), true);
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

  struct BackgroundGouraud
  {
    BoxGouraud topLeft;
    BoxGouraud topRight;
    BoxGouraud bottomRight;
    BoxGouraud bottomLeft;
  };

  static BackgroundGouraud makeBackgroundCircle(const gl::SRGB8& color, uint8_t innerAlpha, uint8_t outerAlpha)
  {
    return makeBackgroundCircle(gl::SRGBA8{color.channels[0], color.channels[1], color.channels[2], innerAlpha},
                                gl::SRGBA8{color.channels[0], color.channels[1], color.channels[2], outerAlpha});
  }

  static BackgroundGouraud makeBackgroundCircle(const gl::SRGBA8& center, const gl::SRGBA8& outer)
  {
    return {ui::BoxGouraud{outer, outer, center, outer},
            ui::BoxGouraud{outer, outer, outer, center},
            ui::BoxGouraud{center, outer, outer, outer},
            ui::BoxGouraud{outer, center, outer, outer}};
  }

  bool blink = false;
  Alignment alignX = Alignment::Left;
  Alignment alignY = Alignment::Top;
  bool fillBackground = false;
  std::optional<BackgroundGouraud> backgroundGouraud;
  bool outline = false;
  glm::ivec2 pos{0};
  int16_t letterSpacing = 1;
  int16_t wordSpacing = 6;
  int16_t blinkTime = 0;
  mutable int16_t timeout = 0;
  glm::ivec2 bgndSize{0};
  glm::ivec2 bgndOff{0};
  int scale = FontBaseScale;
  std::string text;

  explicit Label(const glm::ivec2& pos, const std::string& string)
      : pos{pos}
      , text{string, 0, std::min(std::string::size_type(64), string.size())}
  {
  }

  void draw(const CachedFont& font, gl::Image<gl::SRGBA8>& img, const loader::file::Palette& palette) const;

  int calcWidth() const;

  void addBackground(const glm::ivec2& size, const glm::ivec2& off)
  {
    bgndSize = size;
    bgndOff = off;
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
