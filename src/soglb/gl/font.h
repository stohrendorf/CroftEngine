#pragma once

#include "freetype/freetype.h"
#include "freetype/fttypes.h"
#include "pixel.h"
#include "soglb_fwd.h"

#include <cstdint>
#include <filesystem>
#include <freetype/ftcache.h>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <string>
#include <utility>

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

  void drawText(
    Image<PremultipliedSRGBA8>& img, gsl::czstring text, glm::ivec2 xy, const PremultipliedSRGBA8& color, int size);
  void drawText(Image<PremultipliedSRGBA8>& img,
                const std::string& text,
                const glm::ivec2& xy,
                uint8_t red,
                uint8_t green,
                uint8_t blue,
                uint8_t alpha,
                int size);

  void drawText(Image<ScalarByte>& img, gsl::czstring text, glm::ivec2 xy, int size);
  void drawText(Image<ScalarByte>& img, const std::string& text, const glm::ivec2& xy, int size);

  [[nodiscard]] std::pair<glm::ivec2, glm::ivec2> measure(gsl::czstring text, int size);
  [[nodiscard]] std::pair<glm::ivec2, glm::ivec2> measure(const std::string& text, int size);

  int getGlyphKernAdvance(FT_UInt left, FT_UInt right) const;

  FT_UInt getGlyphIndex(char32_t chr) const;

private:
  FTC_Manager m_cache = nullptr;
  mutable FTC_CMapCache m_cmapCache = nullptr;
  mutable FTC_SBitCache m_sbitCache = nullptr;
  float m_lineHeight{0};

  std::filesystem::path m_filename;
  FT_Face getFace() const;
};
} // namespace gl
