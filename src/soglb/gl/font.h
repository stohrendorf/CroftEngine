#pragma once

#include "image.h"
#include "pixel.h"

#include <ft2build.h>
#include FT_CACHE_H

#include <filesystem>
#include <optional>
#include <utility>

namespace gl
{
class Font
{
public:
  enum class VAlign
  {
    Top,
    Middle,
    Baseline,
    Bottom
  };

  enum class HAlign
  {
    Left,
    Center,
    Right
  };

  Font(const Font&) = delete;
  Font(Font&&) noexcept = delete;
  Font& operator=(const Font&) = delete;
  Font& operator=(Font&&) = delete;

  void drawText(Image<SRGBA8>& img, gsl::czstring text, int x, int y, const SRGBA8& color, int size);
  glm::ivec2 getBounds(gsl::czstring text, int size) const;
  void drawText(Image<SRGBA8>& img,
                const std::string& text,
                int x,
                int y,
                uint8_t red,
                uint8_t green,
                uint8_t blue,
                uint8_t alpha,
                int size);

  explicit Font(std::filesystem::path ttf);
  ~Font();

  FT_Size_Metrics getMetrics();
  int getGlyphKernAdvance(FT_UInt left, FT_UInt right) const;

  FT_UInt getGlyphIndex(char32_t chr) const;

  glm::ivec2 getAlign(gsl::czstring text, HAlign hAlign, VAlign vAlign, int size) const
  {
    const auto x = getHAlign(text, hAlign, size);
    const auto y = getVAlign(vAlign, size);
    return glm::ivec2{x, y};
  }

  int getVAlign(VAlign align, int size) const
  {
    Expects(size > 0);
    switch(align)
    {
    case VAlign::Top: return std::lround(m_ascender * float(size));
    case VAlign::Middle: return std::lround((m_ascender + m_descender) * float(size) / 2.0f);
    case VAlign ::Baseline: return 0;
    case VAlign::Bottom: return std::lround(m_descender * float(size));
    }
  }

  int getHAlign(gsl::czstring text, HAlign align, int size) const
  {
    switch(align)
    {
    case HAlign::Left: return 0;
    case HAlign::Right: return -getBounds(text, size).x;
    case HAlign::Center: return getBounds(text, size).x / 2;
    }
  }

private:
  FTC_Manager m_cache = nullptr;
  mutable FTC_CMapCache m_cmapCache = nullptr;
  mutable FTC_SBitCache m_sbitCache = nullptr;
  float m_ascender{0};
  float m_descender{0};
  float m_lineHeight{0};

  const std::filesystem::path m_filename;
  FT_Face getFace() const;
};

class DisplayFont
{
public:
  explicit DisplayFont(std::shared_ptr<Font> font, std::string text)
      : m_font(std::move(font))
      , m_text{std::move(text)}
  {
    Expects(m_font != nullptr);
  }

  DisplayFont& size(int size)
  {
    m_size = size;
    return *this;
  }

  DisplayFont& align(Font::HAlign hAlign, Font::VAlign vAlign)
  {
    Expects(m_size > 0);
    m_align = m_font->getAlign(m_text.c_str(), hAlign, vAlign, m_size);
    return *this;
  }

  DisplayFont& color(const gl::SRGBA8& color)
  {
    m_color = color;
    return *this;
  }

  void draw(Image<SRGBA8>& img, int x, int y) const
  {
    Expects(m_align.has_value());
    Expects(m_size > 0);
    Expects(m_color.has_value());

    m_font->drawText(img, m_text.c_str(), x + m_align.value().x, y + m_align.value().y, m_color.value(), m_size);
  }

private:
  const std::shared_ptr<Font> m_font;
  const std::string m_text;
  int m_size = 0;
  std::optional<glm::ivec2> m_align;
  std::optional<gl::SRGBA8> m_color;
};
} // namespace gl
