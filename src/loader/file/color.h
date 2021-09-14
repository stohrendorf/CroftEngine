#pragma once

#include <array>
#include <cstdint>
#include <gl/pixel.h>
#include <glm/vec3.hpp>
#include <gsl/gsl-lite.hpp>
#include <memory>

namespace loader::file
{
namespace io
{
class SDLReader;
}

/**
* @brief 8-Bit RGBA color.
* @ingroup native
*
* @note The alpha component is set from TR2 on only.
*/
struct ByteColor
{
  uint8_t r = 0, g = 0, b = 0, a = 0;

  static ByteColor readTr1(io::SDLReader& reader)
  {
    return read(reader, false);
  }

  static ByteColor readTr2(io::SDLReader& reader)
  {
    return read(reader, true);
  }

  [[nodiscard]] gl::SRGBA8 toTextureColor() const
  {
    return gl::SRGBA8{r, g, b, a};
  }

  [[nodiscard]] gl::SRGBA8 toTextureColor(uint8_t alphaOverride) const
  {
    return gl::SRGBA8{r, g, b, alphaOverride};
  }

  [[nodiscard]] glm::vec3 toGLColor3() const
  {
    return glm::vec3{
      gsl::narrow_cast<float>(r) / 255.0f, gsl::narrow_cast<float>(g) / 255.0f, gsl::narrow_cast<float>(b) / 255.0f};
  }

private:
  static ByteColor read(io::SDLReader& reader, bool withAlpha);
};

/**
* @brief 32-Bit float RGBA color.
* @ingroup native
*/
struct FloatColor
{
  float r, g, b, a;
};

struct Palette
{
  std::array<ByteColor, 256> colors;

  /// \brief reads the 256 color palette values.
  static std::unique_ptr<Palette> readTr1(io::SDLReader& reader);

  static std::unique_ptr<Palette> readTr2(io::SDLReader& reader);
};
} // namespace loader::file
