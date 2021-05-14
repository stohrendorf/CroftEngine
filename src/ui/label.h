#pragma once

#include "boxgouraud.h"
#include "engine/world/sprite.h"
#include "loader/file/color.h"

#include <cstdint>
#include <optional>
#include <string>

namespace loader::file
{
struct Palette;
} // namespace loader::file

namespace ui
{
class Ui;

extern std::string makeAmmoString(const std::string& str);

constexpr int FontBaseScale = 0x10000;

class TRFont
{
  gsl::span<const engine::world::Sprite> m_sprites;
  const int m_scale;

public:
  explicit TRFont(const engine::world::SpriteSequence& sequence, const int scale = FontBaseScale)
      : m_sprites{sequence.sprites}
      , m_scale{scale}
  {
  }

  void draw(ui::Ui& ui, size_t n, const glm::ivec2& xy) const;

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

    [[nodiscard]] auto withAlpha(uint8_t alpha) const
    {
      return BackgroundGouraud{
        topLeft.withAlpha(alpha), topRight.withAlpha(alpha), bottomRight.withAlpha(alpha), bottomLeft.withAlpha(alpha)};
    }
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
  uint8_t backgroundAlpha = 0;
  std::optional<BackgroundGouraud> backgroundGouraud;
  uint8_t backgroundGouraudAlpha = 0;
  uint8_t outlineAlpha = 0;
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

  void draw(Ui& ui, const TRFont& font, const glm::ivec2& screenSize) const;

  int calcWidth() const;

  void addBackground(const glm::ivec2& size, const glm::ivec2& off)
  {
    bgndSize = size;
    bgndOff = off;
    backgroundAlpha = 255;
  }

  void flashText(bool newBlink, int16_t newBlinkTime)
  {
    blink = newBlink;
    if(newBlink)
    {
      blinkTime = newBlinkTime;
      timeout = newBlinkTime;
    }
  }

  [[nodiscard]] glm::ivec2 getOrigin(const glm::ivec2& screenSize) const;
};
} // namespace ui
