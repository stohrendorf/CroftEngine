#include "blockdata.hpp"

#include "bitmap.hpp"
#include "math.hpp"
#include "processrgb.hpp"
#include "ssevec.h"
#include "tables.hpp"

#include <array>
#include <boost/iostreams/device/mapped_file.hpp>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <ios>
#include <memory>

#ifdef WIN32
#  include <intrin.h>
#  define _bswap(x) _byteswap_ulong(x)
#  define _bswap64(x) _byteswap_uint64(x)
#else
#  include <x86intrin.h>
#endif

#ifndef _bswap
#  define _bswap(x) __builtin_bswap32(x)
#endif
#ifndef _bswap64
#  define _bswap64(x) __builtin_bswap64(x)
#endif

namespace
{
constexpr const std::array<uint8_t, 8> table59T58H{{3, 6, 11, 16, 23, 32, 41, 64}};

boost::iostreams::mapped_file_sink openForWriting(const char* fn, size_t len, const glm::ivec2& size)
{
  {
    std::ofstream tmp{fn, std::ios::binary | std::ios::trunc};
    tmp.seekp(len - 1, std::ios::beg);
    const char zero = 0;
    tmp.write(&zero, 1);
  }

  const boost::iostreams::mapped_file_sink sink{fn, len};
  gsl_Assert(sink.is_open());
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto dst = reinterpret_cast<uint32_t*>(sink.data());

  *dst++ = 0x03525650; // version
  *dst++ = 0;          // flags
  *dst++ = 23;         // pixelformat[0]
  *dst++ = 0;          // pixelformat[1]
  *dst++ = 0;          // colourspace
  *dst++ = 0;          // channel type
  *dst++ = size.y;     // height
  *dst++ = size.x;     // width
  *dst++ = 1;          // depth
  *dst++ = 1;          // num surfs
  *dst++ = 1;          // num faces
  *dst++ = 1;          // mipmap count
  *dst++ = 0;          // metadata size

  return sink;
}
} // namespace

BlockData::BlockData(const char* fn)
    : m_file{std::make_unique<boost::iostreams::mapped_file_sink>(fn)}
{
  gsl_Assert(m_file->is_open());
  m_maplen = m_file->size();
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  m_data = reinterpret_cast<uint8_t*>(m_file->data());

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto data32 = reinterpret_cast<uint32_t*>(m_data);
  gsl_Assert(*data32 == 0x03525650);
  // pixelformat[0]
  gsl_Assert(*(data32 + 2) == 23);

  m_size.y = gsl::narrow_cast<int32_t>(*(data32 + 6));
  m_size.x = gsl::narrow_cast<int32_t>(*(data32 + 7));
  gsl_Assert(m_size.x > 0 && m_size.y > 0);
  m_dataOffset = 52u + *(data32 + 12);
}

BlockData::BlockData(const char* fn, const glm::ivec2& size)
    : m_size(size)
    , m_dataOffset(52)
    , m_maplen(gsl::narrow_cast<size_t>((m_size.x / 4) * (m_size.y / 4)) * sizeof(uint64_t) * 2u)
{
  gsl_Expects(m_size.x % 4 == 0 && m_size.y % 4 == 0);

  m_maplen += m_dataOffset;
  m_file = std::make_unique<boost::iostreams::mapped_file_sink>(openForWriting(fn, m_maplen, m_size));
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  m_data = reinterpret_cast<uint8_t*>(m_file->data());
}

BlockData::BlockData(const glm::ivec2& size)
    : m_size(size)
    , m_dataOffset(52)
    , m_maplen{gsl::narrow_cast<size_t>((m_size.x / 4) * (m_size.y / 4)) * sizeof(uint64_t) * 2u}
{
  gsl_Assert(m_size.x > 0 && m_size.y > 0);
  gsl_Assert(m_size.x % 4 == 0 && m_size.y % 4 == 0);

  m_maplen += m_dataOffset;
  m_data = new uint8_t[m_maplen];
}

BlockData::~BlockData()
{
  if(m_file == nullptr)
    delete[] m_data;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void BlockData::processRgba(const uint32_t* src, uint32_t blocks, size_t offset, size_t width, bool useHeuristics)
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto dst = reinterpret_cast<uint64_t*>(m_data + m_dataOffset) + offset * 2u;

  compressEtc2Bgra(src, dst, blocks, width, useHeuristics);
}

namespace
{
constexpr uint8_t expand6(uint8_t value)
{
  // NOLINTNEXTLINE(hicpp-signed-bitwise)
  return (value << 2u) | (value >> 4u);
}

constexpr uint8_t expand7(uint8_t value)
{
  // NOLINTNEXTLINE(hicpp-signed-bitwise)
  return (value << 1u) | (value >> 6u);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void decodeTAlpha(uint64_t block, uint64_t alpha, uint32_t* dst, uint32_t w)
{
  const auto r0 = (block >> 24u) & 0x1Bu;
  const auto rh0 = (r0 >> 3u) & 0x3u;
  const auto rl0 = r0 & 0x3u;
  const auto g0 = (block >> 20u) & 0xFu;
  const auto b0 = (block >> 16u) & 0xFu;

  const auto r1 = (block >> 12u) & 0xFu;
  const auto g1 = (block >> 8u) & 0xFu;
  const auto b1 = (block >> 4u) & 0xFu;

  const auto cr0 = gsl::narrow_cast<uint8_t>(((rh0 << 6u) | (rl0 << 4u) | (rh0 << 2u) | rl0));
  const auto cg0 = gsl::narrow_cast<uint8_t>((g0 << 4u) | g0);
  const auto cb0 = gsl::narrow_cast<uint8_t>((b0 << 4u) | b0);

  const auto cr1 = gsl::narrow_cast<uint16_t>((r1 << 4u) | r1);
  const auto cg1 = gsl::narrow_cast<uint16_t>((g1 << 4u) | g1);
  const auto cb1 = gsl::narrow_cast<uint16_t>((b1 << 4u) | b1);

  const auto codeword_hi = (block >> 2u) & 0x3u;
  const auto codeword_lo = block & 0x1u;
  const auto codeword = (codeword_hi << 1u) | codeword_lo;

  const auto base = gsl::narrow_cast<int32_t>(alpha >> 56u);
  const auto mul = gsl::narrow_cast<int32_t>((alpha >> 52u) & 0xFu);

  const auto c2r = clampu8(cr1 + table59T58H[codeword]);
  const auto c2g = clampu8(cg1 + table59T58H[codeword]);
  const auto c2b = clampu8(cb1 + table59T58H[codeword]);

  const auto c3r = clampu8(cr1 - table59T58H[codeword]);
  const auto c3g = clampu8(cg1 - table59T58H[codeword]);
  const auto c3b = clampu8(cb1 - table59T58H[codeword]);

  const std::array<uint32_t, 4> col_tab{{
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    gsl::narrow_cast<uint32_t>(cr0 | (cg0 << 8u) | (cb0 << 16u)),
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    gsl::narrow_cast<uint32_t>(c2r | (c2g << 8u) | (c2b << 16u)),
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    gsl::narrow_cast<uint32_t>(cr1 | (cg1 << 8u) | (cb1 << 16u)),
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    gsl::narrow_cast<uint32_t>(c3r | (c3g << 8u) | (c3b << 16u)),
  }};

  const auto& tbl = getAlpha()[(alpha >> 48u) & 0xFu];
  const uint32_t indexes = (block >> 32u) & 0xFFFFFFFFu;
  for(uint8_t j = 0; j < 4; j++)
  {
    for(uint8_t i = 0; i < 4; i++)
    {
      //2bit indices distributed on two lane 16bit numbers
      const uint8_t index = (((indexes >> (j + i * 4u + 16u)) & 0x1u) << 1u) | ((indexes >> (j + i * 4u)) & 0x1u);
      const auto amod = tbl[(alpha >> (45u - j * 3u - i * 12u)) & 0x7u];
      const uint32_t a = clampu8(base + amod * mul);
      dst[j * w + i] = col_tab[index] | (a << 24u);
    }
  }
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void decodeHAlpha(uint64_t block, uint64_t alpha, uint32_t* dst, uint32_t w)
{
  const uint32_t indexes = (block >> 32u) & 0xFFFFFFFFu;

  const auto r0444 = (block >> 27u) & 0xFu;
  const auto g0444 = ((block >> 20u) & 0x1u) | (((block >> 24u) & 0x7u) << 1u);
  const auto b0444 = ((block >> 15u) & 0x7u) | (((block >> 19u) & 0x1u) << 3u);

  const auto r1444 = (block >> 11u) & 0xFu;
  const auto g1444 = (block >> 7u) & 0xFu;
  const auto b1444 = (block >> 3u) & 0xFu;

  const auto r0 = gsl::narrow_cast<uint8_t>((r0444 << 4u) | r0444);
  const auto g0 = gsl::narrow_cast<uint8_t>((g0444 << 4u) | g0444);
  const auto b0 = gsl::narrow_cast<uint8_t>((b0444 << 4u) | b0444);

  const auto r1 = gsl::narrow_cast<uint8_t>((r1444 << 4u) | r1444);
  const auto g1 = gsl::narrow_cast<uint8_t>((g1444 << 4u) | g1444);
  const auto b1 = gsl::narrow_cast<uint8_t>((b1444 << 4u) | b1444);

  const auto codeword_hi = ((block & 0x1u) << 1u) | ((block & 0x4u));
  const auto c0 = (r0444 << 8u) | (g0444 << 4u) | (b0444 << 0u);
  const auto c1 = (block >> 3u) & ((1u << 12u) - 1u);
  const auto codeword_lo = (c0 >= c1) ? 1u : 0u;
  const auto codeword = codeword_hi | codeword_lo;

  const int32_t base = gsl::narrow_cast<uint8_t>(alpha >> 56u);
  const int32_t mul = gsl::narrow_cast<uint8_t>((alpha >> 52u) & 0xFu);
  const auto& tbl = getAlpha()[(alpha >> 48u) & 0xFu];

  const std::array<uint32_t, 4> col_tab{{
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    gsl::narrow_cast<uint32_t>(clampu8(r0 + table59T58H[codeword]) | (clampu8(g0 + table59T58H[codeword]) << 8u)
                               | (clampu8(b0 + table59T58H[codeword]) << 16u)),
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    gsl::narrow_cast<uint32_t>(clampu8(r0 - table59T58H[codeword]) | (clampu8(g0 - table59T58H[codeword]) << 8u)
                               | (clampu8(b0 - table59T58H[codeword]) << 16u)),
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    gsl::narrow_cast<uint32_t>(clampu8(r1 + table59T58H[codeword]) | (clampu8(g1 + table59T58H[codeword]) << 8u)
                               | (clampu8(b1 + table59T58H[codeword]) << 16u)),
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    gsl::narrow_cast<uint32_t>(clampu8(r1 - table59T58H[codeword]) | (clampu8(g1 - table59T58H[codeword]) << 8u)
                               | (clampu8(b1 - table59T58H[codeword]) << 16u)),
  }};

  for(uint8_t j = 0; j < 4; j++)
  {
    for(uint8_t i = 0; i < 4; i++)
    {
      const uint8_t index = (((indexes >> (j + i * 4u + 16u)) & 0x1u) << 1u) | ((indexes >> (j + i * 4u)) & 0x1u);
      const auto amod = tbl[(alpha >> (45u - j * 3u - i * 12u)) & 0x7u];
      const uint32_t a = clampu8(base + amod * mul);
      dst[j * w + i] = col_tab[index] | (a << 24u);
    }
  }
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void decodePlanarAlpha(uint64_t block, uint64_t alpha, uint32_t* dst, uint32_t w)
{
  const auto bv = expand6((block >> (0u + 32u)) & 0x3Fu);
  const auto gv = expand7((block >> (6u + 32u)) & 0x7Fu);
  const auto rv = expand6((block >> (13u + 32u)) & 0x3Fu);

  const auto bh = expand6((block >> (19u + 32u)) & 0x3Fu);
  const auto gh = expand7((block >> (25u + 32u)) & 0x7Fu);

  const auto rh0 = gsl::narrow_cast<uint8_t>((block >> (32u - 32u)) & 0x01u); // cppcheck-suppress duplicateExpression
  const auto rh1 = gsl::narrow_cast<uint8_t>(((block >> (34u - 32u)) & 0x1Fu) << 1u);
  const auto rh = expand6(rh0 | rh1);

  const uint8_t bo0 = (block >> (39u - 32u)) & 0x07u;
  const uint8_t bo1 = ((block >> (43u - 32u)) & 0x3u) << 3u;
  const uint8_t bo2 = ((block >> (48u - 32u)) & 0x1u) << 5u;
  // NOLINTNEXTLINE(hicpp-signed-bitwise)
  const auto bo = expand6(bo0 | bo1 | bo2);
  const uint8_t go0 = (block >> (49u - 32u)) & 0x3Fu;
  const uint8_t go1 = ((block >> (56u - 32u)) & 0x01u) << 6u;
  const auto go = expand7(go0 | go1);
  const auto ro = expand6((block >> (57u - 32u)) & 0x3Fu);

  const auto base = gsl::narrow_cast<int32_t>(alpha >> 56u);
  const auto mul = gsl::narrow_cast<int32_t>((alpha >> 52u) & 0xFu);
  const auto& tbl = getAlpha()[(alpha >> 48u) & 0xFu];

  auto chco = IVec16{
    gsl::narrow_cast<int16_t>(rh - ro),
    gsl::narrow_cast<int16_t>(gh - go),
    gsl::narrow_cast<int16_t>(bh - bo),
    0,
    0,
    0,
    0,
    0,
  };
  auto cvco = IVec16{
    gsl::narrow_cast<int16_t>((rv - ro) - 4u * (rh - ro)),
    gsl::narrow_cast<int16_t>((gv - go) - 4u * (gh - go)),
    gsl::narrow_cast<int16_t>((bv - bo) - 4u * (bh - bo)),
    0,
    0,
    0,
    0,
    0,
  };
  auto col = IVec16{
    gsl::narrow_cast<int16_t>(4u * ro + 2u),
    gsl::narrow_cast<int16_t>(4u * go + 2u),
    gsl::narrow_cast<int16_t>(4u * bo + 2u),
    0,
    0,
    0,
    0,
    0,
  };

  for(uint8_t j = 0; j < 4; j++)
  {
    for(uint8_t i = 0; i < 4; i++)
    {
      const auto amod = tbl[(alpha >> (45u - j * 3u - i * 12u)) & 0x7u];
      const uint32_t a = clampu8(base + amod * mul);
      auto c = col.sra(2u);
      auto s = c.toIVec8U(c);
      dst[j * w + i] = s.get32() | (a << 24u);
      col = col + chco;
    }
    col = col + cvco;
  }
}

uint64_t convertByteOrder(uint64_t d)
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
  std::array<uint32_t, 2> word;
  memcpy(word.data(), &d, 8);
  word[0] = _bswap(word[0]);
  word[1] = _bswap(word[1]);
  memcpy(&d, word.data(), 8);
  return d;
}

void decodeRgbaPart(uint64_t d, uint64_t alpha, uint32_t* dst, uint32_t w)
{
  d = convertByteOrder(d);
  alpha = _bswap64(alpha);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
  std::array<uint32_t, 2> br, bg, bb;

  if(d & 0x2u)
  {
    const auto r0 = gsl::narrow_cast<uint8_t>((d & 0xF8000000u) >> 27u);
    const auto g0 = gsl::narrow_cast<uint8_t>((d & 0x00F80000u) >> 19u);
    const auto b0 = gsl::narrow_cast<uint8_t>((d & 0x0000F800u) >> 11u);

    const auto dr = (int32_t(d) << 5u) >> 29u;
    const auto dg = (int32_t(d) << 13u) >> 29u;
    const auto db = (int32_t(d) << 21u) >> 29u;

    const int32_t r1 = int32_t(r0) + dr;
    const int32_t g1 = int32_t(g0) + dg;
    const int32_t b1 = int32_t(b0) + db;

    // T mode
    if((r1 < 0) || (r1 > 31))
    {
      decodeTAlpha(d, alpha, dst, w);
      return;
    }

    // H mode
    if((g1 < 0) || (g1 > 31))
    {
      decodeHAlpha(d, alpha, dst, w);
      return;
    }

    // P mode
    if((b1 < 0) || (b1 > 31))
    {
      decodePlanarAlpha(d, alpha, dst, w);
      return;
    }

    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    br[0] = (r0 << 3u) | (r0 >> 2u);
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    br[1] = (r1 << 3u) | (r1 >> 2u);
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    bg[0] = (g0 << 3u) | (g0 >> 2u);
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    bg[1] = (g1 << 3u) | (g1 >> 2u);
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    bb[0] = (b0 << 3u) | (b0 >> 2u);
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    bb[1] = (b1 << 3u) | (b1 >> 2u);
  }
  else
  {
    br[0] = ((d & 0xF0000000u) >> 24u) | ((d & 0xF0000000u) >> 28u);
    br[1] = ((d & 0x0F000000u) >> 20u) | ((d & 0x0F000000u) >> 24u);
    bg[0] = ((d & 0x00F00000u) >> 16u) | ((d & 0x00F00000u) >> 20u);
    bg[1] = ((d & 0x000F0000u) >> 12u) | ((d & 0x000F0000u) >> 16u);
    bb[0] = ((d & 0x0000F000u) >> 8u) | ((d & 0x0000F000u) >> 12u);
    bb[1] = ((d & 0x00000F00u) >> 4u) | ((d & 0x00000F00u) >> 8u);
  }

  const std::array<uint32_t, 2> tcw{{
    gsl::narrow_cast<uint32_t>((d & 0xE0u) >> 5u),
    gsl::narrow_cast<uint32_t>((d & 0x1Cu) >> 2u),
  }};

  uint32_t b1 = (d >> 32u) & 0xFFFFu;
  uint32_t b2 = (d >> 48u);

  b1 = (b1 | (b1 << 8u)) & 0x00FF00FFu;
  b1 = (b1 | (b1 << 4u)) & 0x0F0F0F0Fu;
  b1 = (b1 | (b1 << 2u)) & 0x33333333u;
  b1 = (b1 | (b1 << 1u)) & 0x55555555u;

  b2 = (b2 | (b2 << 8u)) & 0x00FF00FFu;
  b2 = (b2 | (b2 << 4u)) & 0x0F0F0F0Fu;
  b2 = (b2 | (b2 << 2u)) & 0x33333333u;
  b2 = (b2 | (b2 << 1u)) & 0x55555555u;

  uint32_t idx = b1 | (b2 << 1u);

  const int32_t base = gsl::narrow_cast<uint8_t>(alpha >> 56u);
  const int32_t mul = gsl::narrow_cast<uint8_t>((alpha >> 52u) & 0xFu);
  const auto& atbl = getAlpha()[(alpha >> 48u) & 0xFu];

  if(d & 0x1u)
  {
    for(uint8_t i = 0; i < 4; i++)
    {
      for(uint8_t j = 0; j < 4; j++)
      {
        const auto mod = getTable()[tcw[j / 2u]][idx & 0x3u];
        const auto r = br[j / 2u] + mod;
        const auto g = bg[j / 2u] + mod;
        const auto b = bb[j / 2u] + mod;
        const auto amod = atbl[(alpha >> (45u - j * 3u - i * 12u)) & 0x7u];
        const uint32_t a = clampu8(base + amod * mul);
        if(((r | g | b) & ~0xFFu) == 0)
        {
          dst[j * w + i] = r | (g << 8u) | (b << 16u) | (a << 24u);
        }
        else
        {
          const auto rc = clampu8(r);
          const auto gc = clampu8(g);
          const auto bc = clampu8(b);
          // NOLINTNEXTLINE(hicpp-signed-bitwise)
          dst[j * w + i] = rc | (gc << 8u) | (bc << 16u) | (a << 24u);
        }
        idx >>= 2u;
      }
    }
  }
  else
  {
    for(uint8_t i = 0; i < 4; i++)
    {
      const auto& tbl = getTable()[tcw[i / 2u]];
      const auto cr = br[i / 2u];
      const auto cg = bg[i / 2u];
      const auto cb = bb[i / 2u];

      for(uint8_t j = 0; j < 4; j++)
      {
        const auto mod = tbl[idx & 0x3u];
        const auto r = cr + mod;
        const auto g = cg + mod;
        const auto b = cb + mod;
        const auto amod = atbl[(alpha >> (45u - j * 3u - i * 12u)) & 0x7u];
        const uint32_t a = clampu8(base + amod * mul);
        if(((r | g | b) & ~0xFFu) == 0)
        {
          dst[j * w + i] = r | (g << 8u) | (b << 16u) | (a << 24u);
        }
        else
        {
          const auto rc = clampu8(r);
          const auto gc = clampu8(g);
          const auto bc = clampu8(b);
          // NOLINTNEXTLINE(hicpp-signed-bitwise)
          dst[j * w + i] = rc | (gc << 8u) | (bc << 16u) | (a << 24u);
        }
        idx >>= 2u;
      }
    }
  }
}
} // namespace

std::shared_ptr<Bitmap> BlockData::decode()
{
  gsl_Assert(m_dataOffset < m_maplen);

  auto ret = std::make_shared<Bitmap>(m_size);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  const auto* src = reinterpret_cast<const uint64_t*>(m_data + m_dataOffset);
  uint32_t* dst = ret->data();

  for(int y = 0; y < m_size.y / 4; y++)
  {
    for(int x = 0; x < m_size.x / 4; x++)
    {
      const auto a = *src++;
      const auto d = *src++;
      decodeRgbaPart(d, a, dst, m_size.x);
      dst += 4;
    }
    dst += m_size.x * 3;
  }

  return ret;
}
