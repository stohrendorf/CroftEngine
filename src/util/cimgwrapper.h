#pragma once

#include "render/gl/pixel.h"

#include <boost/filesystem/path.hpp>
#include <memory>
#include <string>

namespace cimg_library
{
template<typename T>
struct CImg;
}

namespace util
{
class CImgWrapper
{
private:
  std::unique_ptr<cimg_library::CImg<uint8_t>> m_image;
  bool m_interleaved = false;

public:
  explicit CImgWrapper(const std::string& filename);

  explicit CImgWrapper(const boost::filesystem::path& filename)
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

  void resize(int width, int height);

  void resizeHalfMipmap();

  void resizePow2Mipmap(uint8_t n);

  void crop(int x0, int y0, int x1, int y1);

  [[nodiscard]] CImgWrapper cropped(int x0, int y0, int x1, int y1) const;

  void crop(const glm::vec2& uv0, const glm::vec2& uv1);

  void crop(const glm::ivec2& uv0, const glm::ivec2& uv1)
  {
    crop(uv0.x, uv0.y, uv1.x, uv1.y);
  }

  uint8_t& operator()(int x, int y, int c);

  render::gl::SRGBA8& operator()(int x, int y);

  uint8_t operator()(int x, int y, int c) const;

  [[nodiscard]] const uint8_t* data() const;

  void savePng(const std::string& filename);

  void savePng(const boost::filesystem::path& filename)
  {
    savePng(filename.string());
  }

  void replace(int x, int y, const CImgWrapper& other);

private:
  void unshare();
};
} // namespace util