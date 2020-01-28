#pragma once

#include <boost/assert.hpp>
#include <gl/pixel.h>
#include <gsl-lite.hpp>

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

  [[nodiscard]] glm::vec4 toGLColor() const
  {
    return glm::vec4{r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
  }

  [[nodiscard]] glm::vec3 toGLColor3() const
  {
    return glm::vec3{r / 255.0f, g / 255.0f, b / 255.0f};
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

  [[nodiscard]] glm::vec4 toSColor(const float intensity) const
  {
    BOOST_ASSERT(intensity >= 0 && intensity <= 1);
    glm::vec4 col;
    col.x = r * intensity;
    col.y = g * intensity;
    col.z = b * intensity;
    col.w = a * intensity;
    return col;
  }
};

struct Palette
{
  ByteColor colors[256];

  /// \brief reads the 256 color palette values.
  static std::unique_ptr<Palette> readTr1(io::SDLReader& reader);

  static std::unique_ptr<Palette> readTr2(io::SDLReader& reader);
};
} // namespace loader::file
