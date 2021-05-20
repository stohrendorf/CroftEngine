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

class TRFont
{
  gsl::span<const engine::world::Sprite> m_sprites;

public:
  explicit TRFont(const engine::world::SpriteSequence& sequence)
      : m_sprites{sequence.sprites}
  {
  }

  void draw(ui::Ui& ui, size_t n, const glm::ivec2& xy) const;
};

struct Label
{
  static constexpr char NumberSign = '@';
  static constexpr char SzLig = '=';
  static constexpr char SquareBracketLeft = '<';
  static constexpr char SquareBracketRight = '>';
  static constexpr char SmallNumber0 = '\x01';

  static constexpr char Note = '|';
  static constexpr char Speaker = '}';
  static constexpr char ShotgunAmmo = '\x0c';
  static constexpr char MagnumsAmmo = '\x0d';
  static constexpr char UzisAmmo = '\x0e';
  static constexpr char ArrowUp = '[';
  static constexpr char ArrowDown = ']';

  static constexpr char UmlautDots = '~';
  static constexpr char Acute1 = '(';
  static constexpr char Acute2 = ')';

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

  Alignment alignX = Alignment::Left;
  Alignment alignY = Alignment::Top;
  uint8_t backgroundAlpha = 0;
  std::optional<BackgroundGouraud> backgroundGouraud;
  uint8_t backgroundGouraudAlpha = 0;
  uint8_t outlineAlpha = 0;
  glm::ivec2 pos{0};
  glm::ivec2 bgndSize{0};
  std::string text;

  explicit Label(const glm::ivec2& pos, const std::string& string)
      : pos{pos}
      , text{string, 0, std::min(std::string::size_type(64), string.size())}
  {
  }

  void draw(Ui& ui, const TRFont& font, const glm::ivec2& screenSize) const;

  [[nodiscard]] int calcWidth() const;
};
} // namespace ui
