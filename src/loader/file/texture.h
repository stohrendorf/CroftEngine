#pragma once

#include "core/id.h"
#include "qs/qs.h"
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
  std::array<std::array<uint8_t, 256>, 256> pixels;

  static std::unique_ptr<ByteTexture> read(io::SDLReader& reader);
};

struct WordTexture
{
  std::array<std::array<uint16_t, 256>, 256> pixels; //!< R5G5B5A1

  static std::unique_ptr<WordTexture> read(io::SDLReader& reader);
};

struct DWordTexture final
{
  std::array<std::array<gl::SRGBA8, 256>, 256> pixels;
  std::shared_ptr<gl::Image<gl::SRGBA8>> image;

  std::string md5;

  static std::unique_ptr<DWordTexture> read(io::SDLReader& reader);

  void toImage();
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
  QS_DECLARE_QUANTITY(Component, uint16_t, "uv");
  static constexpr float ComponentScale = std::numeric_limits<uint16_t>::max() + 1;

  Component x{};
  Component y{};

  constexpr bool operator==(const UVCoordinates& rhs) const noexcept
  {
    return x == rhs.x && y == rhs.y;
  }

  constexpr bool operator<(const UVCoordinates& rhs) const noexcept
  {
    if(x != rhs.x)
      return x < rhs.x;
    return y < rhs.y;
  }

  static UVCoordinates read(io::SDLReader& reader);

  [[nodiscard]] glm::vec2 toGl() const
  {
    return glm::vec2{x.get<float>() / ComponentScale, y.get<float>() / ComponentScale};
  }

  [[nodiscard]] glm::ivec2 toPx(int w, int h) const
  {
    return glm::ivec2{toGl() * glm::vec2{w, h}};
  }

  [[nodiscard]] glm::ivec2 toPx(int wh) const
  {
    return toPx(wh, wh);
  }

  [[nodiscard]] glm::ivec2 toNearestPx(int size) const
  {
    return glm::round(toGl() * gsl::narrow_cast<float>(size));
  }

  [[nodiscard]] glm::vec2 toNearestGl(int size) const
  {
    return glm::vec2{toNearestPx(size)} / gsl::narrow_cast<float>(size);
  }

  void set(const glm::vec2& uv)
  {
    BOOST_ASSERT(uv.x >= 0 && uv.x < 1);
    BOOST_ASSERT(uv.y >= 0 && uv.y < 1);
    x = Component{gsl::narrow_cast<Component::type>(uv.x * ComponentScale)};
    y = Component{gsl::narrow_cast<Component::type>(uv.y * ComponentScale)};
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

  std::array<UVCoordinates, 4> uvCoordinates{}; // the four corners of the texture
  uint32_t unknown1{};                          // TR4
  uint32_t unknown2{};                          // TR4
  uint32_t x_size{};                            // TR4
  uint32_t y_size{};                            // TR4

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

  [[nodiscard]] std::pair<glm::ivec2, glm::ivec2> getMinMaxPx(int size) const
  {
    glm::ivec2 xy0{std::numeric_limits<glm::int32>::max()};
    glm::ivec2 xy1{std::numeric_limits<glm::int32>::min()};
    for(const auto& uvComponent : uvCoordinates)
    {
      if(uvComponent.x.get() == 0 && uvComponent.y.get() == 0)
        continue;

      const auto px = uvComponent.toPx(size);
      xy0 = glm::min(px, xy0);
      xy1 = glm::max(px, xy1);
    }
    return {xy0, xy1};
  }

  [[nodiscard]] std::pair<glm::vec2, glm::vec2> getMinMaxUv() const
  {
    glm::vec2 xy0{std::numeric_limits<glm::float32>::max()};
    glm::vec2 xy1{std::numeric_limits<glm::float32>::min()};
    for(const auto& uvComponent : uvCoordinates)
    {
      if(uvComponent.x.get() == 0 && uvComponent.y.get() == 0)
        continue;

      xy0 = glm::min(uvComponent.toGl(), xy0);
      xy1 = glm::max(uvComponent.toGl(), xy1);
    }
    return {xy0, xy1};
  }
};
} // namespace file
} // namespace loader
