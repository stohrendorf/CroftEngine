#include "text.h"

#include "ui.h"
#include "util.h"

#include <algorithm>
#include <array>
#include <utility>

namespace ui
{
namespace
{
constexpr int LetterSpacing = 1;
constexpr int WordSpacing = 6;

const std::array<const int, 110> charWidths{
  14, 11, 11, 11, 11, 11, 11, 13, 8,  11, 12, 11, 13, 13, 12, 11, 12, 12, 11, 12, 13, 13, 13, 12, 12, 11, 9,  9,
  9,  9,  9,  9,  9,  9,  5,  9,  9,  5,  12, 10, 9,  9,  9,  8,  9,  8,  9,  9,  11, 9,  9,  9,  12, 8,  10, 10,
  10, 10, 10, 9,  10, 10, 5,  5,  5,  11, 9,  10, 8,  6,  6,  7,  7,  3,  11, 8,  13, 16, 9,  4,  12, 12, 7,  5,
  7,  7,  7,  7,  7,  7,  7,  7,  16, 14, 14, 14, 16, 16, 16, 16, 16, 12, 14, 8,  8,  8,  8,  8,  8,  8};

const std::array<const uint8_t, 98> charToSprite{
  0,  64, 66, 78, 77, 74, 78, 79, 69, 70, 92, 72, 63, 71, 62, 68, 52,  53,  54,  55, 56, 57, 58, 59, 60,
  61, 73, 73, 66, 74, 75, 65, 0,  0,  1,  2,  3,  4,  5,  6,  7,  8,   9,   10,  11, 12, 13, 14, 15, 16,
  17, 18, 19, 20, 21, 22, 23, 24, 25, 80, 76, 81, 97, 98, 77, 26, 27,  28,  29,  30, 31, 32, 33, 34, 35,
  36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 100, 101, 102, 67, 0,  0,  0};

std::vector<std::tuple<glm::ivec2, uint8_t>> doLayout(const std::string& text, int* width = nullptr)
{
  if(text.empty())
  {
    if(width != nullptr)
      *width = 0;
    return {};
  }

  std::vector<std::tuple<glm::ivec2, uint8_t>> layout;

  glm::ivec2 xy{0, 0};
  bool isSpriteSelector = false;
  for(const uint8_t chr : text)
  {
    uint8_t sprite = chr;
    if(chr == SpriteSelector)
    {
      isSpriteSelector = true;
      continue;
    }

    if(isSpriteSelector)
    {
      isSpriteSelector = false;
      sprite = chr;
      layout.emplace_back(xy, sprite);
    }
    else
    {
      if(chr > 15 && chr < 32)
        continue;

      if(chr == ' ')
      {
        xy.x += WordSpacing;
        if(width != nullptr)
          *width = xy.x;
        continue;
      }

      if(chr <= 10)
        sprite = chr + 81;
      else if(chr <= 15)
        sprite = chr + 91;
      else
        sprite = charToSprite.at(chr - 32);

      layout.emplace_back(xy, sprite);

      if(chr == Acute1 || chr == Acute2 || chr == Gravis || chr == UmlautDots)
        continue;
    }

    xy.x += charWidths[sprite] + LetterSpacing;
    if(width != nullptr)
      *width = xy.x;
  }

  if(width != nullptr)
    *width -= LetterSpacing;

  return layout;
}
} // namespace

std::string makeAmmoString(const std::string& str)
{
  std::string result;
  for(const char c : str)
  {
    if(c == ' ')
    {
      result += c;
      continue;
    }

    if(c < 'A')
    {
      result += static_cast<char>(int(c) - int('0') + 1);
    }
    else
    {
      result += static_cast<char>(int(c) - int('A') + 12);
    }
  }
  return result;
}

void TRFont::draw(ui::Ui& ui, size_t sprite, const glm::ivec2& xy) const
{
  ui.draw(m_sprites[sprite], xy);
}

Text::Text(const std::string& text)
    : m_layout{doLayout(text, &m_width)}
{
  Ensures(m_width >= 0);
}

void Text::draw(Ui& ui, const TRFont& font, const glm::ivec2& position) const
{
  for(const auto& [xy, sprite] : m_layout)
  {
    font.draw(ui, sprite, xy + position);
  }
}
} // namespace ui
