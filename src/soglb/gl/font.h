#pragma once

#include "pixel.h"
#include "soglb_fwd.h"

#include <ft2build.h>
#include FT_CACHE_H

#include <boost/throw_exception.hpp>
#include <filesystem>
#include <glm/common.hpp>
#include <optional>

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

  void drawText(Image<SRGBA8>& img, gsl::czstring text, glm::ivec2 xy, const SRGBA8& color, int size);
  glm::ivec2 getBounds(gsl::czstring text, int size) const;
  void drawText(Image<SRGBA8>& img,
                const std::string& text,
                const glm::ivec2& xy,
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

  glm::ivec2 getAlign(const gsl::czstring text, const HAlign hAlign, const VAlign vAlign, const int size) const
  {
    const auto x = getHAlign(text, hAlign, size);
    const auto y = getVAlign(vAlign, size);
    return glm::ivec2{x, y};
  }

  int getVAlign(const VAlign align, const int size) const
  {
    Expects(size > 0);
    switch(align)
    {
    case VAlign::Top: return std::lround(m_ascender * static_cast<float>(size));
    case VAlign::Middle: return std::lround((m_ascender + m_descender) * static_cast<float>(size) / 2.0f);
    case VAlign ::Baseline: return 0;
    case VAlign::Bottom: return std::lround(m_descender * static_cast<float>(size));
    default: BOOST_THROW_EXCEPTION(std::domain_error("Invalid Alignment"));
    }
  }

  int getHAlign(const gsl::czstring text, const HAlign align, const int size) const
  {
    switch(align)
    {
    case HAlign::Left: return 0;
    case HAlign::Right: return -getBounds(text, size).x;
    case HAlign::Center: return getBounds(text, size).x / 2;
    default: BOOST_THROW_EXCEPTION(std::domain_error("Invalid Alignment"));
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
} // namespace gl
