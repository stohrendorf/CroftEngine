#include "cimgwrapper.h"

#ifdef _X
#  undef _X
#endif

#include "pixel.h"
#include "texture2d.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/assert.hpp>
#include <CImg.h>
#include <fstream>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <utility>

namespace gl
{
namespace
{
struct PcxHeader
{
  uint8_t zsoft;
  uint8_t version;
  uint8_t encoding;
  uint8_t bpp;
  uint16_t minX;
  uint16_t minY;
  uint16_t maxX;
  uint16_t maxY;
  uint16_t dpiX;
  uint16_t dpiY;
  RGB8 colorMap[16];
  uint8_t _reserved;
  uint8_t planes;
  uint16_t bytesPerLine;
  uint16_t paletteType;
  uint16_t width;
  uint16_t height;
  uint8_t _pad[54];
};
static_assert(sizeof(PcxHeader) == 128);
} // namespace

CImgWrapper::CImgWrapper(const std::string& filename)
    : m_image{boost::algorithm::to_lower_copy(std::filesystem::path{filename}.extension().string()) == ".pcx"
                ? loadPcx(filename)
                : std::make_unique<cimg_library::CImg<uint8_t>>(filename.c_str())}
{
  if(m_image->spectrum() == 3)
  {
    m_image->channels(0, 3);
    BOOST_ASSERT(m_image->spectrum() == 4);
    m_image->get_shared_channel(3).fill(255);
  }

  gsl_Ensures(m_image->spectrum() == 4);
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
    : CImgWrapper{size, size}
{
}

CImgWrapper::CImgWrapper(const int w, const int h)
    : m_image{std::make_unique<cimg_library::CImg<uint8_t>>(w, h, 1, 4)}
{
  m_image->fill(0);
}

CImgWrapper::CImgWrapper()
    : m_image{nullptr}
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

void CImgWrapper::resize(const glm::ivec2& size)
{
  unshare();
  if(!m_interleaved)
    m_image->resize(size.x, size.y, 1, 4, 6, 1);
  else
    m_image->resize(4, size.x, size.y, 1, 6, 1);
}

void CImgWrapper::crop(const int x0, const int y0, const int x1, const int y1)
{
  unshare();
  if(!m_interleaved)
    m_image->crop(x0, y0, 0, 0, x1, y1, 0, 3);
  else
    m_image->crop(0, x0, y0, 0, 3, x1, y1, 0);
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
  return reinterpret_cast<SRGBA8&>((*m_image)(0, x, y, 0)); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
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

void CImgWrapper::savePng(const std::string& filename, bool premultiplied)
{
  deinterleave();
  if(!premultiplied)
  {
    m_image->save_png(filename.c_str(), 1);
    return;
  }

  const auto tmp = std::make_unique<cimg_library::CImg<int>>(*m_image, false);
  tmp->get_shared_channel(0).mul(tmp->get_shared_channel(3)) /= 255;
  tmp->get_shared_channel(1).mul(tmp->get_shared_channel(3)) /= 255;
  tmp->get_shared_channel(2).mul(tmp->get_shared_channel(3)) /= 255;
  tmp->save_png(filename.c_str(), 1);
}

void CImgWrapper::unshare()
{
  if(m_image->is_shared())
    m_image = std::make_unique<cimg_library::CImg<uint8_t>>(*m_image, false);
}

void CImgWrapper::replace(const int x, const int y, const CImgWrapper& other)
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

void CImgWrapper::extendBorder(int margin)
{
  gsl_Expects(margin >= 0);
  unshare();
  if(!m_interleaved)
    m_image->resize(width() + 2 * margin, height() + 2 * margin, -100, -100, 0, 1, 0.5f, 0.5f, 0, 0);
  else
    m_image->resize(-100, width() + 2 * margin, height() + 2 * margin, -100, 0, 1, 0, 0.5f, 0.5f, 0);
}

void CImgWrapper::fromScreenshot()
{
  deinterleave();
  m_image->get_shared_channel(3).fill(255);
  m_image->mirror('y');
  interleave();
}

gslu::nn_shared<gl::Texture2D<gl::PremultipliedSRGBA8>> CImgWrapper::toTexture(const std::string_view& label)
{
  auto result = gsl::make_shared<gl::Texture2D<gl::PremultipliedSRGBA8>>(glm::ivec2{width(), height()}, label);
  result->assign(premultipliedPixels());
  return result;
}

std::unique_ptr<cimg_library::CImg<uint8_t>> CImgWrapper::loadPcx(const std::filesystem::path& filename)
{
  std::ifstream stream{filename, std::ios::in | std::ios::binary};
  gsl_Assert(stream.is_open());

  PcxHeader header;
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  stream.read(reinterpret_cast<char*>(&header), sizeof(PcxHeader));

  gsl_Assert(header.zsoft == 10 && header.version >= 5 && header.bpp == 8 && header.encoding == 1
             && header.planes == 1);
  gsl_Assert(header.maxX >= header.minX);
  gsl_Assert(header.maxY >= header.minY);

  auto img
    = std::make_unique<cimg_library::CImg<uint8_t>>(header.maxX + 1 - header.minX, header.maxY + 1 - header.minY, 1, 4);
  auto remainingPixels = img->width() * img->height();
  img->permute_axes("cxyz");
  auto px = reinterpret_cast<SRGBA8*>(img->data());
  gsl_Assert(px != nullptr);

  using Color = uint8_t[3];
  using Palette = Color[256];
  static_assert(sizeof(Palette) == 3 * 256);
  Palette palette;
  stream.seekg(-static_cast<std::ifstream::pos_type>(sizeof(Palette)), std::ios::end);
  gsl_Assert(stream.good());
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  stream.read(reinterpret_cast<char*>(&palette[0][0]), sizeof(Palette));
  gsl_Assert(stream.good());

  auto readByte = [&stream]()
  {
    uint8_t tmp;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    stream.read(reinterpret_cast<char*>(&tmp), 1);
    gsl_Assert(stream.good());
    return tmp;
  };

  stream.seekg(sizeof(PcxHeader), std::ios::beg);
  while(remainingPixels > 1)
  {
    uint8_t repeat = readByte();
    Color* c;
    if((repeat & 0xc0u) == 0xc0u)
    {
      c = &palette[readByte()];
      repeat &= ~0xc0u;
    }
    else
    {
      c = &palette[repeat];
      repeat = 1;
    }

    gsl_Assert(repeat <= remainingPixels);
    while(repeat > 0)
    {
      *px++ = gl::SRGBA8{(*c)[0], (*c)[1], (*c)[2], 255};
      --remainingPixels;
      --repeat;
    }
  }

  img->permute_axes("yzcx");
  return img;
}
} // namespace gl
