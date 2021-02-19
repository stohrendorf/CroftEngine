#include "label.h"

#include "render/scene/material.h"
#include "ui.h"

#include <gl/cimgwrapper.h>

namespace ui
{
namespace
{
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

int Label::calcWidth() const
{
  int width = 0;

  for(uint8_t chr : text)
  {
    if(chr > 129 || (chr > 10 && chr < 32))
      continue;
    if(chr == '(' || chr == ')' || chr == '$' || chr == '~')
      continue;

    if(chr == ' ')
    {
      width += (wordSpacing * scale) / FontBaseScale;
      continue;
    }

    if(chr <= 10)
      chr += 81;
    else if(chr < 16)
      chr += 91;
    else
      chr = charToSprite[chr - ' '];

    width += (charWidths[chr] + letterSpacing) * scale / FontBaseScale;
  }

  width -= letterSpacing;
  return width;
}

void Label::render(Ui& ui, const CachedFont& font, const glm::ivec2& screenSize) const
{
  Expects(font.getScale() == scale);

  if(blink)
  {
    --timeout;
    if(timeout <= -blinkTime)
    {
      timeout = blinkTime;
    }
    else if(timeout < 0)
    {
      return;
    }
  }

  auto xy = pos;
  const auto textWidth = calcWidth();

  if(alignX == Alignment::Center)
  {
    xy.x += (screenSize.x - textWidth) / 2;
  }
  else if(alignX == Alignment::Right)
  {
    xy.x += screenSize.x - textWidth;
  }

  if(alignY == Alignment::Center)
  {
    xy.y += screenSize.y / 2;
  }
  else if(alignY == Alignment::Bottom)
  {
    xy.y += screenSize.y;
  }

  auto bgnd = bgndOff + xy - glm::ivec2{2, 15};
  glm::ivec2 effectiveBgndSize{0};
  if(fillBackground || outline)
  {
    if(bgndSize.x != 0)
    {
      effectiveBgndSize.x = bgndSize.x + 4;
      bgnd.x += (textWidth - bgndSize.x) / 2;
    }
    else
    {
      effectiveBgndSize.x = textWidth + 4;
    }

    if(bgndSize.y != 0)
    {
      effectiveBgndSize.y = bgndSize.y;
    }
    else
    {
      effectiveBgndSize.y = 16;
    }
  }

  if(fillBackground)
  {
    if(!backgroundGouraud.has_value())
    {
      ui.drawBox(bgnd, effectiveBgndSize, {0, 0, 0, 192});
    }
    else
    {
      const auto half = effectiveBgndSize / 2;
      const auto half2 = effectiveBgndSize - half;
      const auto& g = backgroundGouraud.value();
      ui.drawBox(bgnd, half, g.topLeft);
      ui.drawBox(bgnd + glm::ivec2{half.x, 0}, {half2.x, half.y}, g.topRight);
      ui.drawBox(bgnd + half, {half.x, half2.y}, g.bottomRight);
      ui.drawBox(bgnd + glm::ivec2{0, half.y}, {half2.x, half2.y}, g.bottomLeft);
    }
  }

  for(uint8_t chr : text)
  {
    const auto origChar = chr;
    if(chr > 15 && chr < 32)
      continue;

    if(chr == ' ')
    {
      xy.x += (wordSpacing * scale) / FontBaseScale;
      continue;
    }

    if(chr <= 10)
      chr += 81;
    else if(chr <= 15)
      chr += 91;
    else
      chr = charToSprite.at(chr - 32);

    font.render(chr, xy, screenSize);

    if(origChar == '(' || origChar == ')' || origChar == '$' || origChar == '~')
      continue;

    xy.x += (charWidths[chr] + letterSpacing) * scale / FontBaseScale;
  }

  if(outline)
  {
    ui.drawOutlineBox(bgnd, effectiveBgndSize);
  }
}

void CachedFont::render(size_t n, const glm::ivec2& xy, const glm::ivec2& screenSize) const
{
  ScreenSprite screenSprite{m_sprites[n]};
  screenSprite.render(xy, screenSize, m_material);
}
} // namespace ui
