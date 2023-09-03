#include "text.h"

#include "core.h"
#include "ui.h"
#include "util.h"

#include <array>
#include <boost/algorithm/string/trim.hpp>
#include <gl/pixel.h>
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

void TRFont::draw(ui::Ui& ui, size_t sprite, const glm::ivec2& xy, float scale, float alpha) const
{
  ui.draw(m_sprites[sprite], xy, scale, alpha);
}

Text::Text(const std::string& text)
    : m_layout{doLayout(text, &m_width)}
{
  gsl_Ensures(m_width >= 0);
}

void Text::draw(Ui& ui, const TRFont& font, const glm::ivec2& position, float scale, float alpha) const
{
  for(const auto& [xy, sprite] : m_layout)
  {
    font.draw(ui, sprite, xy + position, scale, alpha);
  }
}

void drawBox(const Text& text, Ui& ui, const glm::ivec2& pos, int padding, const gl::SRGBA8& color, float scale)
{
  ui.drawBox(pos + glm::ivec2{-padding, padding},
             glm::ivec2{text.getWidth() * scale + 2 * padding, -FontHeight * scale - 2 * padding - 2},
             color);
}

std::vector<std::string> breakLines(const std::string& text, int maxWidth)
{
  std::vector<std::string> words;
  size_t start = 0;
  while(start != std::string::npos)
  {
    const auto last = text.find_first_of("\n ", start);
    if(last == std::string::npos)
    {
      words.emplace_back(text.substr(start));
      break;
    }
    if(last != start)
      words.emplace_back(text.substr(start, last - start));
    words.emplace_back(text.substr(last, 1));
    start = last + 1;
  }

  std::vector<std::string> lines;
  std::string candidate;
  bool hadImplicitLineBreak = false;
  for(const auto& word : words)
  {
    if(word == "\n")
    {
      // forced newline; trim trailing spaces
      if(!hadImplicitLineBreak)
        lines.emplace_back(boost::algorithm::trim_right_copy(candidate));
      candidate.clear();
      hadImplicitLineBreak = false;
      continue;
    }
    else if(word == " ")
    {
      // skip spaces at start of line
      if(!candidate.empty())
        candidate += ' ';
      hadImplicitLineBreak = false;
      continue;
    }

    auto nextCandidate = candidate + word;

    int width = 0;
    doLayout(nextCandidate, &width);
    if(width <= maxWidth)
    {
      candidate = std::move(nextCandidate);
      hadImplicitLineBreak = false;
    }
    else
    {
      // line would exceed max length, add previous
      lines.emplace_back(boost::algorithm::trim_right_copy(candidate));
      candidate = word;
      hadImplicitLineBreak = true;
    }
  }

  candidate = boost::algorithm::trim_right_copy(candidate);
  if(!candidate.empty())
  {
    lines.emplace_back(std::move(candidate));
  }

  return lines;
}
} // namespace ui
