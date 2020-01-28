#pragma once

#include "core/id.h"
#include "render/scene/material.h"

#include <boost/lexical_cast.hpp>
#include <gl/image.h>

namespace loader
{
namespace trx
{
class Glidos;
}

namespace file
{
namespace io
{
class SDLReader;
}

struct ByteTexture
{
  uint8_t pixels[256][256];

  static std::unique_ptr<ByteTexture> read(io::SDLReader& reader);
};

struct WordTexture
{
  uint16_t pixels[256][256]; //!< R5G5B5A1

  static std::unique_ptr<WordTexture> read(io::SDLReader& reader);
};

struct DWordTexture final
{
  gl::SRGBA8 pixels[256][256];
  std::shared_ptr<gl::Image<gl::SRGBA8>> image;

  std::string md5;

  static std::unique_ptr<DWordTexture> read(io::SDLReader& reader);

  void toImage(const trx::Glidos* glidos, const std::function<void(const std::string&)>& statusCallback);
};

enum class BlendingMode : uint16_t
{
  Solid,
  AlphaTransparency,
  VertexColorTransparency,
  SimpleShade,
  TransparentIgnoreZ,
  InvertSrc,
  Wireframe,
  TransparentAlpha,
  InvertDst,
  Screen,
  Hide,
  AnimatedTexture
};

struct UVCoordinates
{
  int8_t xcoordinate; // 1 if Xpixel is the low value, -1 if Xpixel is the high value in the object texture
  uint8_t xpixel;

  int8_t ycoordinate; // 1 if Ypixel is the low value, -1 if Ypixel is the high value in the object texture
  uint8_t ypixel;

  constexpr bool operator==(const UVCoordinates& rhs) const noexcept
  {
    return xcoordinate == rhs.xcoordinate && xpixel == rhs.xpixel && ycoordinate == rhs.ycoordinate
           && ypixel == rhs.ypixel;
  }

  constexpr bool operator<(const UVCoordinates& rhs) const noexcept
  {
    if(xcoordinate != rhs.xcoordinate)
      return xcoordinate < rhs.xcoordinate;
    if(xpixel != rhs.xpixel)
      return xpixel < rhs.xpixel;
    if(ycoordinate != rhs.ycoordinate)
      return ycoordinate < rhs.ycoordinate;
    return ypixel < rhs.ypixel;
  }

  static UVCoordinates readTr1(io::SDLReader& reader);

  static UVCoordinates readTr4(io::SDLReader& reader);

  [[nodiscard]] glm::vec2 toGl() const
  {
    return glm::vec2{(static_cast<float>(xpixel) + 0.5f) / 256.0f, (static_cast<float>(ypixel) + 0.5f) / 256.0f};
  }
};

struct TextureKey
{
  BlendingMode blendingMode = BlendingMode::Solid;

  // 0 means that a texture is all-opaque, and that transparency
  // information is ignored.
  // 1 means that transparency information is used. In 8-bit color,
  // index 0 is the transparent color, while in 16-bit color, the
  // top bit (0x8000) is the alpha channel (1 = opaque, 0 = transparent).
  // 2 (only in TR3) means that the opacity (alpha) is equal to the intensity;
  // the brighter the color, the more opaque it is. The intensity is probably calculated
  // as the maximum of the individual color values.
  uint16_t tileAndFlag = 0; // index into textile list

  uint16_t flags = 0; // TR4

  DECLARE_ID(ColorId, int);
  ColorId colorId{-1};

  bool operator==(const TextureKey& rhs) const
  {
    return tileAndFlag == rhs.tileAndFlag && flags == rhs.flags && blendingMode == rhs.blendingMode
           && colorId == rhs.colorId;
  }

  bool operator<(const TextureKey& rhs) const
  {
    if(tileAndFlag != rhs.tileAndFlag)
    {
      return tileAndFlag < rhs.tileAndFlag;
    }

    if(flags != rhs.flags)
    {
      return flags < rhs.flags;
    }

    if(blendingMode != rhs.blendingMode)
    {
      return blendingMode < rhs.blendingMode;
    }

    return colorId.get() < rhs.colorId.get();
  }
};

struct TextureTile
{
  TextureKey textureKey;

  std::array<UVCoordinates, 4> uvCoordinates; // the four corners of the texture
  uint32_t unknown1;                          // TR4
  uint32_t unknown2;                          // TR4
  uint32_t x_size;                            // TR4
  uint32_t y_size;                            // TR4

  bool operator==(const TextureTile& rhs) const
  {
    return textureKey == rhs.textureKey && uvCoordinates == rhs.uvCoordinates && unknown1 == rhs.unknown1
           && unknown2 == rhs.unknown2 && x_size == rhs.x_size && y_size == rhs.y_size;
  }

  bool operator<(const TextureTile& rhs) const
  {
    if(!(textureKey == rhs.textureKey))
      return textureKey < rhs.textureKey;

    if(unknown1 != rhs.unknown1)
      return unknown1 < rhs.unknown1;

    if(unknown2 != rhs.unknown2)
      return unknown2 < rhs.unknown2;

    if(x_size != rhs.x_size)
      return x_size < rhs.x_size;

    return y_size < rhs.y_size;
  }

  static std::unique_ptr<TextureTile> readTr1(io::SDLReader& reader);

  static std::unique_ptr<TextureTile> readTr4(io::SDLReader& reader);

  static std::unique_ptr<TextureTile> readTr5(io::SDLReader& reader);
};
} // namespace file
} // namespace loader
