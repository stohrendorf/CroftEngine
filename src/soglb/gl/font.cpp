#include "font.h"

#include "image.h"

#include <boost/log/trivial.hpp>
#include <freetype/ftoutln.h>
#include <gsl/gsl-lite.hpp>
#include <utf8.h>
#include <utility>

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

  Expects(freeTypeLib != nullptr);

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
  Ensures(m_cache != nullptr);

  error = FTC_CMapCache_New(m_cache, &m_cmapCache);
  if(error != FT_Err_Ok)
  {
    BOOST_LOG_TRIVIAL(fatal) << "Failed to create cmap cache: " << getFreeTypeErrorMessage(error);
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create cmap cache"));
  }
  Ensures(m_cmapCache != nullptr);

  error = FTC_SBitCache_New(m_cache, &m_sbitCache);
  if(error != FT_Err_Ok)
  {
    BOOST_LOG_TRIVIAL(fatal) << "Failed to create cmap cache: " << getFreeTypeErrorMessage(error);
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create cmap cache"));
  }
  Ensures(m_sbitCache != nullptr);

  const auto face = getFace();
  const auto h = face->ascender - face->descender;
  Expects(h != 0);
  m_ascender = static_cast<float>(face->ascender) / h;
  m_descender = static_cast<float>(face->descender) / h;
  m_lineHeight = static_cast<float>(face->height) / h;
}

Font::~Font()
{
  FTC_Manager_Done(m_cache);
  m_cache = nullptr;
}

void Font::drawText(Image<SRGBA8>& img, const gsl::czstring text, glm::ivec2 xy, const SRGBA8& color, int size)
{
  Expects(text);
  Expects(size > 0);

  size *= m_lineHeight;

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
    if(error != FT_Err_Ok || sbit->buffer == nullptr)
    {
      BOOST_LOG_TRIVIAL(warning) << "Failed to load from sbit cache: " << getFreeTypeErrorMessage(error);
      FTC_Node_Unref(node, m_cache);
      continue;
    }

    for(int dy = 0, i = 0; dy < sbit->height; dy++)
    {
      for(int dx = 0; dx < sbit->width; dx++, i++)
      {
        currentColor.channels[3] = gsl::narrow_cast<uint8_t>(sbit->buffer[i] * baseAlpha / 255);
        img.set(xy + glm::ivec2{dx + sbit->left, dy - sbit->top}, currentColor, true);
      }
    }

    if(prevChar.has_value())
      xy.x += getGlyphKernAdvance(prevChar.value(), chr);

    xy.x += sbit->xadvance;
    xy.y += sbit->yadvance;

    FTC_Node_Unref(node, m_cache);
    prevChar = chr;
  }
}

glm::ivec2 Font::getBounds(const gsl::czstring text, int size) const
{
  Expects(text);
  Expects(size > 0);

  size = std::lround(m_lineHeight * static_cast<float>(size));

  FTC_ImageTypeRec imgType;
  imgType.face_id = const_cast<Font*>(this); // NOLINT(cppcoreguidelines-pro-type-const-cast)
  imgType.width = size;
  imgType.height = size;
  imgType.flags = FT_LOAD_DEFAULT | FT_LOAD_RENDER; // NOLINT(hicpp-signed-bitwise)

  int x = 0;
  int y = size;

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

    if(prevChar.has_value())
      x += getGlyphKernAdvance(prevChar.value(), chr);
    x += sbit->xadvance;
    y += sbit->yadvance;

    FTC_Node_Unref(node, m_cache);
    prevChar = chr;
  }

  return glm::ivec2{x, y};
}

void Font::drawText(Image<SRGBA8>& img,
                    const std::string& text,
                    const glm::ivec2& xy,
                    const uint8_t red,
                    const uint8_t green,
                    const uint8_t blue,
                    const uint8_t alpha,
                    const int size)
{
  drawText(img, text.c_str(), xy, SRGBA8{red, green, blue, alpha}, size);
}

FT_Size_Metrics Font::getMetrics()
{
  FT_Face face;
  if(FTC_Manager_LookupFace(m_cache, this, &face) != FT_Err_Ok)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to retrieve face information"));
  }

  return face->size->metrics;
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
  return std::lround(k.x / 64.0f * m_lineHeight);
}

FT_UInt Font::getGlyphIndex(const char32_t chr) const
{
  return FTC_CMapCache_Lookup(m_cmapCache,
                              const_cast<Font*>(this), // NOLINT(cppcoreguidelines-pro-type-const-cast)
                              -1,
                              chr);
}
} // namespace gl
