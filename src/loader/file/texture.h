#pragma once

#include "core/id.h"
#include "qs/qs.h"

#include <array>
#include <boost/assert.hpp>
#include <cstdint>
#include <gl/image.h>
#include <gl/pixel.h>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <limits>
#include <memory>
#include <string>

namespace loader::file
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
  mutable std::shared_ptr<gl::Image<gl::SRGBA8>> image;

  std::string md5;

  static std::unique_ptr<DWordTexture> read(io::SDLReader& reader);

  void toImage() const;
};

// NOLINTNEXTLINE(*-enum-size)
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
  static constexpr float ComponentScale = static_cast<float>(std::numeric_limits<uint16_t>::max()) + 1;

  // Valid coordinates are 1..65535; UV coordinates 0..1 are mapped to 0..65536
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

  /**
   * Converts the TR texture coordinates to OpenGL coordinates.
   * @return UV coordinates in ]0..1[.
   */
  [[nodiscard]] glm::vec2 toGl() const
  {
    return glm::vec2{x.get<float>() / ComponentScale, y.get<float>() / ComponentScale};
  }

  void set(const glm::vec2& uv)
  {
    BOOST_ASSERT(uv.x >= 0 && uv.x < 1);
    BOOST_ASSERT(uv.y >= 0 && uv.y < 1);
    x = Component{gsl::narrow_cast<Component::type>(uv.x * ComponentScale)};
    y = Component{gsl::narrow_cast<Component::type>(uv.y * ComponentScale)};
  }

  [[nodiscard]] constexpr bool isUnset() const
  {
    return x.get() == 0 && y.get() == 0;
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
  uint16_t atlasIdAndFlag = 0; // index into textile list

  uint16_t flags = 0; // TR4

  DECLARE_ID(ColorId, int);
  ColorId colorId{-1};

  bool operator==(const TextureKey& rhs) const
  {
    return blendingMode == rhs.blendingMode && atlasIdAndFlag == rhs.atlasIdAndFlag && flags == rhs.flags
           && colorId == rhs.colorId;
  }

  bool operator<(const TextureKey& rhs) const
  {
    if(blendingMode != rhs.blendingMode)
    {
      return blendingMode < rhs.blendingMode;
    }

    if(atlasIdAndFlag != rhs.atlasIdAndFlag)
    {
      return atlasIdAndFlag < rhs.atlasIdAndFlag;
    }

    if(flags != rhs.flags)
    {
      return flags < rhs.flags;
    }

    return colorId.get() < rhs.colorId.get();
  }
};

struct TextureTile
{
  TextureKey textureKey;

  std::array<UVCoordinates, 4> uvCoordinates{}; // the four corners of the texture
  uint32_t unknown1 = 0;                        // TR4
  uint32_t unknown2 = 0;                        // TR4
  uint32_t x_size = 0;                          // TR4
  uint32_t y_size = 0;                          // TR4

  [[nodiscard]] bool isOpaque() const noexcept
  {
    return textureKey.blendingMode == BlendingMode::Solid;
  }

  static std::unique_ptr<TextureTile> readTr1(io::SDLReader& reader);

  static std::unique_ptr<TextureTile> readTr4(io::SDLReader& reader);

  static std::unique_ptr<TextureTile> readTr5(io::SDLReader& reader);
};
} // namespace loader::file
