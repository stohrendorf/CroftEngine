#include "font.h"

#include "image.h"

#include <algorithm>
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <cstdlib>
#include <cstring>
#include <gl/pixel.h>
#include <gsl/gsl-lite.hpp>
#include <iterator>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <utf8.h> // IWYU pragma: keep
#include <utility>
#include <vector>

namespace gl
{
namespace
{
FT_Library freeTypeLib = nullptr;

gsl::czstring getFreeTypeErrorMessage(const FT_Error err)
{
#undef __FTERRORS_H__
#define FT_ERRORDEF(e, v, s) \
  case e:                    \
    return s;
#define FT_ERROR_START_LIST \
  switch(err)               \
  {
#define FT_ERROR_END_LIST }

#include <freetype/fterrors.h>
  return "(Unknown error)";
}

FT_Library loadFreeTypeLib()
{
  if(freeTypeLib != nullptr)
    return freeTypeLib;

  const auto error = FT_Init_FreeType(&freeTypeLib);
  if(error != FT_Err_Ok)
  {
    BOOST_LOG_TRIVIAL(fatal) << "Failed to load freetype library: " << getFreeTypeErrorMessage(error);
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to load freetype library"));
  }

  gsl_Ensures(freeTypeLib != nullptr);

  atexit(
    []()
    {
      FT_Done_FreeType(freeTypeLib);
      freeTypeLib = nullptr;
    });

  return freeTypeLib;
}
} // namespace

FT_Error ftcFaceRequester(const FTC_FaceID /*face_id*/,
                          const FT_Library library,  // NOLINT(misc-misplaced-const)
                          const FT_Pointer req_data, // NOLINT(misc-misplaced-const)
                          FT_Face* aface)
{
  const auto* path = static_cast<std::filesystem::path*>(req_data);

  const auto error = FT_New_Face(library, path->string().c_str(), 0, aface);
  if(error != FT_Err_Ok)
  {
    BOOST_LOG_TRIVIAL(fatal) << "Failed to load font " << *path << ": " << getFreeTypeErrorMessage(error);
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to load font"));
  }
  return FT_Err_Ok;
}

Font::Font(std::filesystem::path ttf)
    : m_filename{std::move(ttf)}
{
  BOOST_LOG_TRIVIAL(debug) << "Loading font " << m_filename;
  auto error
    = FTC_Manager_New(loadFreeTypeLib(),
                      0,
                      0,
                      0,
                      &ftcFaceRequester,
                      const_cast<std::filesystem::path*>(&m_filename), // NOLINT(cppcoreguidelines-pro-type-const-cast)
                      &m_cache);
  if(error != FT_Err_Ok)
  {
    BOOST_LOG_TRIVIAL(fatal) << "Failed to create cache manager: " << getFreeTypeErrorMessage(error);
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create cache manager"));
  }
  gsl_Ensures(m_cache != nullptr);

  error = FTC_CMapCache_New(m_cache, &m_cmapCache);
  if(error != FT_Err_Ok)
  {
    BOOST_LOG_TRIVIAL(fatal) << "Failed to create cmap cache: " << getFreeTypeErrorMessage(error);
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create cmap cache"));
  }
  gsl_Ensures(m_cmapCache != nullptr);

  error = FTC_SBitCache_New(m_cache, &m_sbitCache);
  if(error != FT_Err_Ok)
  {
    BOOST_LOG_TRIVIAL(fatal) << "Failed to create cmap cache: " << getFreeTypeErrorMessage(error);
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create cmap cache"));
  }
  gsl_Ensures(m_sbitCache != nullptr);

  const auto face = getFace();
  const auto h = face->ascender - face->descender;
  gsl_Assert(h != 0);
  m_lineHeight = gsl::narrow_cast<float>(face->height) / gsl::narrow_cast<float>(h);
}

Font::~Font()
{
  FTC_Manager_Done(m_cache);
  m_cache = nullptr;
}

void Font::drawText(
  Image<PremultipliedSRGBA8>& img, const gsl::czstring text, glm::ivec2 xy, const PremultipliedSRGBA8& color, int size)
{
  gsl_Expects(text != nullptr);
  gsl_Expects(size > 0);

  size = gsl::narrow_cast<int>(gsl::narrow_cast<float>(size) * m_lineHeight);

  const int baseAlpha = color.channels[3];
  auto currentColor = color;

  FTC_ImageTypeRec imgType;
  imgType.face_id = this;
  imgType.width = size;
  imgType.height = size;
  imgType.flags = FT_LOAD_DEFAULT | FT_LOAD_RENDER; // NOLINT(hicpp-signed-bitwise)

  std::optional<char32_t> prevChar = std::nullopt;
  std::vector<char32_t> utf32;
  utf8::utf8to32(text, text + std::strlen(text), std::back_inserter(utf32));
  for(const char32_t chr : utf32)
  {
    const auto glyphIndex = getGlyphIndex(chr);
    if(glyphIndex == 0)
    {
      BOOST_LOG_TRIVIAL(warning) << "Failed to load character '" << chr << "'";
      continue;
    }

    FTC_SBit sbit = nullptr;
    FTC_Node node = nullptr;
    const auto error = FTC_SBitCache_Lookup(m_sbitCache, &imgType, glyphIndex, &sbit, &node);
    if(error != FT_Err_Ok)
    {
      BOOST_LOG_TRIVIAL(warning) << "Failed to load from sbit cache: " << getFreeTypeErrorMessage(error);
      FTC_Node_Unref(node, m_cache);
      continue;
    }

    if(sbit->buffer != nullptr)
    {
      for(int dy = 0, i = 0; dy < sbit->height; dy++)
      {
        for(int dx = 0; dx < sbit->width; dx++, i++)
        {
          currentColor.channels[3] = gsl::narrow_cast<uint8_t>(sbit->buffer[i] * baseAlpha / 255);
          img.set(xy + glm::ivec2{dx + sbit->left, dy - sbit->top}, currentColor, true);
        }
      }
    }

    if(prevChar.has_value())
      xy.x += getGlyphKernAdvance(*prevChar, chr);

    xy.x += sbit->xadvance;
    xy.y += sbit->yadvance;

    FTC_Node_Unref(node, m_cache);
    prevChar = chr;
  }
}

void Font::drawText(Image<PremultipliedSRGBA8>& img,
                    const std::string& text,
                    const glm::ivec2& xy,
                    const uint8_t red,
                    const uint8_t green,
                    const uint8_t blue,
                    const uint8_t alpha,
                    const int size)
{
  drawText(img, text.c_str(), xy, premultiply(SRGBA8{red, green, blue, alpha}), size);
}

void Font::drawText(Image<ScalarByte>& img, const gsl::czstring text, glm::ivec2 xy, int size)
{
  Expects(text != nullptr);
  Expects(size > 0);

  size = gsl::narrow_cast<int>(gsl::narrow_cast<float>(size) * m_lineHeight);

  FTC_ImageTypeRec imgType;
  imgType.face_id = this;
  imgType.width = size;
  imgType.height = size;
  imgType.flags = FT_LOAD_DEFAULT | FT_LOAD_RENDER; // NOLINT(hicpp-signed-bitwise)

  std::optional<char32_t> prevChar = std::nullopt;
  std::vector<char32_t> utf32;
  utf8::utf8to32(text, text + std::strlen(text), std::back_inserter(utf32));
  for(const char32_t chr : utf32)
  {
    const auto glyphIndex = getGlyphIndex(chr);
    if(glyphIndex == 0)
    {
      BOOST_LOG_TRIVIAL(warning) << "Failed to load character '" << chr << "'";
      continue;
    }

    FTC_SBit sbit = nullptr;
    FTC_Node node = nullptr;
    const auto error = FTC_SBitCache_Lookup(m_sbitCache, &imgType, glyphIndex, &sbit, &node);
    if(error != FT_Err_Ok)
    {
      BOOST_LOG_TRIVIAL(warning) << "Failed to load from sbit cache: " << getFreeTypeErrorMessage(error);
      FTC_Node_Unref(node, m_cache);
      continue;
    }

    if(sbit->buffer != nullptr)
    {
      for(int dy = 0, i = 0; dy < sbit->height; dy++)
      {
        for(int dx = 0; dx < sbit->width; dx++, i++)
        {
          img.set(xy + glm::ivec2{dx + sbit->left, dy - sbit->top},
                  ScalarByte{gsl::narrow_cast<uint8_t>(sbit->buffer[i])});
        }
      }
    }

    if(prevChar.has_value())
      xy.x += getGlyphKernAdvance(*prevChar, chr);

    xy.x += sbit->xadvance;
    xy.y += sbit->yadvance;

    FTC_Node_Unref(node, m_cache);
    prevChar = chr;
  }
}

void Font::drawText(Image<ScalarByte>& img, const std::string& text, const glm::ivec2& xy, const int size)
{
  drawText(img, text.c_str(), xy, size);
}

std::pair<glm::ivec2, glm::ivec2> Font::measure(const gsl::czstring text, int size)
{
  Expects(text != nullptr);
  Expects(size > 0);

  size = gsl::narrow_cast<int>(gsl::narrow_cast<float>(size) * m_lineHeight);

  FTC_ImageTypeRec imgType;
  imgType.face_id = this;
  imgType.width = size;
  imgType.height = size;
  imgType.flags = FT_LOAD_DEFAULT | FT_LOAD_RENDER; // NOLINT(hicpp-signed-bitwise)

  glm::ivec2 bottomLeft{0, 0};
  glm::ivec2 topRight{0, 0};

  glm::ivec2 xy{0, 0};

  std::optional<char32_t> prevChar = std::nullopt;
  std::vector<char32_t> utf32;
  utf8::utf8to32(text, text + std::strlen(text), std::back_inserter(utf32));
  for(const char32_t chr : utf32)
  {
    const auto glyphIndex = getGlyphIndex(chr);
    if(glyphIndex == 0)
    {
      BOOST_LOG_TRIVIAL(warning) << "Failed to load character '" << chr << "'";
      continue;
    }

    FTC_SBit sbit = nullptr;
    FTC_Node node = nullptr;
    const auto error = FTC_SBitCache_Lookup(m_sbitCache, &imgType, glyphIndex, &sbit, &node);
    if(error != FT_Err_Ok)
    {
      BOOST_LOG_TRIVIAL(warning) << "Failed to load from sbit cache: " << getFreeTypeErrorMessage(error);
      FTC_Node_Unref(node, m_cache);
      continue;
    }

    bottomLeft.x = std::min(bottomLeft.x, xy.x + sbit->left);
    bottomLeft.y = std::min(bottomLeft.y, xy.y - sbit->top);
    topRight.x = std::max(topRight.x, xy.x + sbit->left + sbit->width);
    topRight.y = std::max(topRight.y, xy.y + sbit->top + sbit->height);

    if(prevChar.has_value())
      xy.x += getGlyphKernAdvance(*prevChar, chr);

    xy.x += sbit->xadvance;
    xy.y += sbit->yadvance;

    FTC_Node_Unref(node, m_cache);
    prevChar = chr;
  }

  return {bottomLeft, topRight};
}

std::pair<glm::ivec2, glm::ivec2> Font::measure(const std::string& text, const int size)
{
  return measure(text.c_str(), size);
}

FT_Face Font::getFace() const
{
  FT_Face face = nullptr;
  if(FTC_Manager_LookupFace(m_cache,
                            const_cast<Font*>(this), // NOLINT(cppcoreguidelines-pro-type-const-cast)
                            &face)
     != FT_Err_Ok)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to retrieve face information"));
  }
  return face;
}

int Font::getGlyphKernAdvance(const FT_UInt left, const FT_UInt right) const
{
  FT_Vector k{};
  if(FT_Get_Kerning(getFace(), left, right, FT_KERNING_DEFAULT, &k) != FT_Err_Ok)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to retrieve kerning information"));
  }
  return gsl::narrow<int>(std::lround(gsl::narrow<float>(k.x) / 64.0f * m_lineHeight));
}

FT_UInt Font::getGlyphIndex(const char32_t chr) const
{
  return FTC_CMapCache_Lookup(m_cmapCache,
                              const_cast<Font*>(this), // NOLINT(cppcoreguidelines-pro-type-const-cast)
                              -1,
                              chr);
}
} // namespace gl
