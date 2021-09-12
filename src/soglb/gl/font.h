#pragma once

#include "pixel.h"
#include "soglb_fwd.h"

#include <boost/throw_exception.hpp>
#include <filesystem>
#include <freetype/ftcache.h>
#include <glm/common.hpp>
#include <optional>

namespace gl
{
class Font
{
public:
  explicit Font(std::filesystem::path ttf);
  ~Font();

  Font(const Font&) = delete;
  Font(Font&&) noexcept = delete;
  Font& operator=(const Font&) = delete;
  Font& operator=(Font&&) = delete;

  void drawText(Image<SRGBA8>& img, gsl::czstring text, glm::ivec2 xy, const SRGBA8& color, int size);
  void drawText(Image<SRGBA8>& img,
                const std::string& text,
                const glm::ivec2& xy,
                uint8_t red,
                uint8_t green,
                uint8_t blue,
                uint8_t alpha,
                int size);

  int getGlyphKernAdvance(FT_UInt left, FT_UInt right) const;

  FT_UInt getGlyphIndex(char32_t chr) const;

private:
  FTC_Manager m_cache = nullptr;
  mutable FTC_CMapCache m_cmapCache = nullptr;
  mutable FTC_SBitCache m_sbitCache = nullptr;
  float m_lineHeight{0};

  const std::filesystem::path m_filename;
  FT_Face getFace() const;
};
} // namespace gl
