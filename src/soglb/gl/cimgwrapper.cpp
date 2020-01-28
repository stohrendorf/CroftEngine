#include "cimgwrapper.h"

#ifdef _X
#  undef _X
#endif

#include <CImg.h>
#include <boost/assert.hpp>
#include <gsl-lite.hpp>

namespace gl
{
CImgWrapper::CImgWrapper(const std::string& filename)
    : m_image{std::make_unique<cimg_library::CImg<uint8_t>>(filename.c_str())}
{
  if(m_image->spectrum() == 3)
  {
    m_image->channels(0, 3);
    BOOST_ASSERT(m_image->spectrum() == 4);
    m_image->get_shared_channel(3).fill(255);
  }

  Expects(m_image->spectrum() == 4);
}

CImgWrapper::CImgWrapper(const CImgWrapper& other)
    : m_image{std::make_unique<cimg_library::CImg<uint8_t>>(*other.m_image)}
    , m_interleaved{other.m_interleaved}
{
}

CImgWrapper::CImgWrapper(const uint8_t* data, int width, int height, bool shared)
    : m_image{std::make_unique<cimg_library::CImg<uint8_t>>(data, 4, width, height, 1, shared)}
    , m_interleaved{true}
{
}

CImgWrapper::CImgWrapper(const int size)
    : m_image{std::make_unique<cimg_library::CImg<uint8_t>>(size, size, 1, 4)}
    , m_interleaved{false}
{
  m_image->fill(0);
}

CImgWrapper::CImgWrapper(const int w, const int h)
    : m_image{std::make_unique<cimg_library::CImg<uint8_t>>(w, h, 1, 4)}
    , m_interleaved{false}
{
  m_image->fill(0);
}

CImgWrapper::CImgWrapper()
    : m_image{nullptr}
    , m_interleaved{false}
{
}

CImgWrapper& CImgWrapper::operator=(const CImgWrapper& other)
{
  m_image = std::make_unique<cimg_library::CImg<uint8_t>>(*other.m_image);
  m_interleaved = other.m_interleaved;
  return *this;
}

CImgWrapper::~CImgWrapper() = default;

void CImgWrapper::interleave()
{
  if(m_interleaved)
    return;

  unshare();

  m_interleaved = true;
  m_image->permute_axes("cxyz");
}

void CImgWrapper::deinterleave()
{
  if(!m_interleaved)
    return;

  unshare();

  m_interleaved = false;
  m_image->permute_axes("yzcx");
}

int CImgWrapper::width() const
{
  return m_interleaved ? m_image->height() : m_image->width();
}

int CImgWrapper::height() const
{
  return m_interleaved ? m_image->depth() : m_image->height();
}

void CImgWrapper::resize(const int width, const int height)
{
  unshare();
  if(!m_interleaved)
    m_image->resize(width, height, 1, 4, 6);
  else
    m_image->resize(4, width, height, 1, 6);
}

void CImgWrapper::resizeHalfMipmap()
{
  resizePow2Mipmap(1);
}

void CImgWrapper::resizePow2Mipmap(uint8_t n)
{
  unshare();
  const int d = 1 << n;
  if(!m_interleaved)
    m_image->resize(width() / d, height() / d, 1, 4, 6, 1);
  else
    m_image->resize(4, width() / d, height() / d, 1, 6, 1);
}

void CImgWrapper::crop(const int x0, const int y0, const int x1, const int y1)
{
  unshare();
  if(!m_interleaved)
    m_image->crop(x0, y0, 0, 0, x1, y1, 0, 3);
  else
    m_image->crop(0, x0, y0, 0, 3, x1, y1, 0);
}

CImgWrapper CImgWrapper::cropped(const int x0, const int y0, const int x1, const int y1) const
{
  CImgWrapper result{};
  result.m_interleaved = m_interleaved;
  if(!m_interleaved)
    result.m_image = std::make_unique<cimg_library::CImg<uint8_t>>(m_image->get_crop(x0, y0, 0, 0, x1, y1, 0, 3));
  else
    result.m_image = std::make_unique<cimg_library::CImg<uint8_t>>(m_image->get_crop(0, x0, y0, 0, 3, x1, y1, 0));
  result.unshare();
  return result;
}

uint8_t& CImgWrapper::operator()(const int x, const int y, const int c)
{
  BOOST_ASSERT(x >= 0 && x < width());
  BOOST_ASSERT(y >= 0 && y < height());
  if(!m_interleaved)
    return (*m_image)(x, y, 0, c);
  else
    return (*m_image)(c, x, y, 0);
}

SRGBA8& CImgWrapper::operator()(const int x, const int y)
{
  BOOST_ASSERT(x >= 0 && x < width());
  BOOST_ASSERT(y >= 0 && y < height());
  interleave();
  return reinterpret_cast<SRGBA8&>((*m_image)(0, x, y, 0));
}

uint8_t CImgWrapper::operator()(const int x, const int y, const int c) const
{
  if(!m_interleaved)
    return (*m_image)(x, y, 0, c);
  else
    return (*m_image)(c, x, y, 0);
}

const uint8_t* CImgWrapper::data() const
{
  return m_image->data();
}

void CImgWrapper::savePng(const std::string& filename)
{
  deinterleave();
  // ReSharper disable once CppExpressionWithoutSideEffects
  m_image->save_png(filename.c_str(), 1);
}

void CImgWrapper::unshare()
{
  if(m_image->is_shared())
    m_image = std::make_unique<cimg_library::CImg<uint8_t>>(*m_image, false);
}

void CImgWrapper::replace(int x, int y, const CImgWrapper& other)
{
  if(other.m_interleaved)
    interleave();
  else
    deinterleave();

  if(!m_interleaved)
    m_image->draw_image(x, y, 0, 0, *other.m_image);
  else
    m_image->draw_image(0, x, y, 0, *other.m_image);
}

void CImgWrapper::crop(const glm::vec2& uv0, const glm::vec2& uv1)
{
  const auto x0 = static_cast<int>(uv0.x * width());
  const auto y0 = static_cast<int>(uv0.y * height());
  const auto x1 = static_cast<int>(uv1.x * width());
  const auto y1 = static_cast<int>(uv1.y * height());
  crop(x0, y0, x1 + 1, y1 + 1);
}

CImgWrapper::CImgWrapper(CImgWrapper&& other) noexcept
    : m_image{std::move(other.m_image)}
    , m_interleaved{other.m_interleaved}
{
}

CImgWrapper& CImgWrapper::operator=(CImgWrapper&& other) noexcept
{
  m_image = std::exchange(other.m_image, std::make_unique<cimg_library::CImg<uint8_t>>(0u, 0u, 1u, 4u));
  m_interleaved = std::exchange(other.m_interleaved, false);

  return *this;
}
} // namespace gl
