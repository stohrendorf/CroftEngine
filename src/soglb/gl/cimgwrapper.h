#pragma once

#include "pixel.h"
#include "soglb_fwd.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace cimg_library
{
template<typename T>
struct CImg;
}

namespace gl
{
class CImgWrapper
{
private:
  std::unique_ptr<cimg_library::CImg<uint8_t>> m_image;
  bool m_interleaved = false;

public:
  explicit CImgWrapper(const std::string& filename);

  explicit CImgWrapper(const std::filesystem::path& filename)
      : CImgWrapper{filename.string()}
  {
  }

  explicit CImgWrapper(const uint8_t* data, int width, int height, bool shared);

  explicit CImgWrapper(int w, int h);

  explicit CImgWrapper(int size);

  explicit CImgWrapper();

  CImgWrapper(const CImgWrapper& other);

  CImgWrapper(CImgWrapper&& other) noexcept;

  CImgWrapper& operator=(const CImgWrapper& other);

  CImgWrapper& operator=(CImgWrapper&& other) noexcept;

  ~CImgWrapper();

  void interleave();

  void deinterleave();

  [[nodiscard]] int width() const;

  [[nodiscard]] int height() const;

  void resize(const glm::ivec2& size);

  void crop(int x0, int y0, int x1, int y1);

  uint8_t& operator()(int x, int y, int c);

  SRGBA8& operator()(int x, int y);

  uint8_t operator()(int x, int y, int c) const;

  [[nodiscard]] const uint8_t* data() const;

  [[nodiscard]] gsl::span<const gl::SRGBA8> pixels()
  {
    static_assert(sizeof(gl::SRGBA8) == 4);
    interleave();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return gsl::make_span(reinterpret_cast<const gl::SRGBA8*>(data()),
                          gsl::narrow<size_t>(width()) * gsl::narrow<size_t>(height()));
  }

  [[nodiscard]] gsl::span<const gl::PremultipliedSRGBA8> asPremultipliedPixels()
  {
    static_assert(sizeof(gl::PremultipliedSRGBA8) == 4);
    interleave();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return gsl::make_span(reinterpret_cast<const gl::PremultipliedSRGBA8*>(data()),
                          gsl::narrow<size_t>(width()) * gsl::narrow<size_t>(height()));
  }

  void premultiplyPixels()
  {
    interleave();
    for(auto& px : gsl::make_span(const_cast<gl::SRGBA8*>(reinterpret_cast<const gl::SRGBA8*>(data())),
                                  gsl::narrow<size_t>(width()) * gsl::narrow<size_t>(height())))
    {
      px.channels = premultiply(px).channels;
    }
  }

  void savePng(const std::string& filename, bool premultiply);

  void savePng(const std::filesystem::path& filename, bool premultiply)
  {
    savePng(filename.string(), premultiply);
  }

  void replace(int x, int y, const CImgWrapper& other);

  void extendBorder(int margin);

  void fromScreenshot();

  gslu::nn_shared<gl::Texture2D<gl::PremultipliedSRGBA8>> toTexture(const std::string_view& label);

private:
  void unshare();
  static std::unique_ptr<cimg_library::CImg<uint8_t>> loadPcx(const std::filesystem::path& filename);
};
} // namespace gl
