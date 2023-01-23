#include "processrgb.hpp"

#include "forceinline.hpp"
#include "math.hpp"
#include "processcommon.hpp"
#include "ssevec.h"
#include "tables.hpp"

#include <array>
#include <glm/ext/vector_uint3_sized.hpp>
#include <glm/vec3.hpp>
#include <gsl/gsl-lite.hpp>
#include <limits>

#ifdef _MSC_VER
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
using BgraBlockImm = std::array<IVec8, 4u>;

constexpr uint32_t MaxError = 1065369600; // ((38+76+14) * 255)^2
// common T-/H-mode table
constexpr std::array<uint8_t, 8> tableTH{{3, 6, 11, 16, 23, 32, 41, 64}};

// thresholds for the early compression-mode decision scheme
// default: 0.03, 0.09, and 0.38
constexpr std::array<float, 3> ecmdThreshold{{0.03f, 0.09f, 0.38f}};

constexpr uint8_t ModeUndecided = 0;
constexpr uint8_t ModePlanar = 0x1;
constexpr uint8_t ModeTH = 0x2;

struct BgraVec
{
  uint8_t b;
  uint8_t g;
  uint8_t r;
  uint8_t a;
};
static_assert(sizeof(BgraVec) == 4);

using BgraVecBlock = std::array<BgraVec, size_t(4u * 4u)>;
static_assert(sizeof(BgraVecBlock) == size_t(4u * 4u) * sizeof(BgraVec));

struct BgrVec
{
  uint8_t b;
  uint8_t g;
  uint8_t r;

  [[nodiscard]] constexpr auto operator-(uint8_t rhs) const
  {
    return BgrVec{
      gsl::narrow_cast<uint8_t>(std::max(0, b - rhs)),
      gsl::narrow_cast<uint8_t>(std::max(0, g - rhs)),
      gsl::narrow_cast<uint8_t>(std::max(0, r - rhs)),
    };
  }

  [[nodiscard]] constexpr auto operator+(uint8_t rhs) const
  {
    return BgrVec{
      gsl::narrow_cast<uint8_t>(std::min(255, b - rhs)),
      gsl::narrow_cast<uint8_t>(std::min(255, g - rhs)),
      gsl::narrow_cast<uint8_t>(std::min(255, r - rhs)),
    };
  }

  [[nodiscard]] constexpr auto operator<<(uint8_t rhs) const
  {
    return BgrVec{
      gsl::narrow_cast<uint8_t>(b << rhs),
      gsl::narrow_cast<uint8_t>(g << rhs),
      gsl::narrow_cast<uint8_t>(r << rhs),
    };
  }

  [[nodiscard]] constexpr auto operator|(const BgrVec& rhs) const
  {
    return BgrVec{
      gsl::narrow_cast<uint8_t>(b | rhs.b),
      gsl::narrow_cast<uint8_t>(g | rhs.g),
      gsl::narrow_cast<uint8_t>(r | rhs.r),
    };
  }
};

struct Luma
{
  uint8_t max = 0, min = 255, maxIdx = 0, minIdx = 0;
  std::array<uint8_t, 16> val;
};

// slightly faster than std::sort
void insertionSort(std::array<uint8_t, 16>& arr1, std::array<uint8_t, 16>& arr2)
{
  for(uint8_t i = 1; i < 16; ++i)
  {
    const uint8_t value = arr1[i];
    uint8_t hole = i;

    for(; hole > 0 && value < arr1[hole - 1]; --hole)
    {
      arr1[hole] = arr1[hole - 1];
      arr2[hole] = arr2[hole - 1];
    }
    arr1[hole] = value;
    arr2[hole] = i;
  }
}

//converts indices from  |a0|a1|e0|e1|i0|i1|m0|m1|b0|b1|f0|f1|j0|j1|n0|n1|c0|c1|g0|g1|k0|k1|o0|o1|d0|d1|h0|h1|l0|l1|p0|p1| previously used by T- and H-modes
//                     into  |p0|o0|n0|m0|l0|k0|j0|i0|h0|g0|f0|e0|d0|c0|b0|a0|p1|o1|n1|m1|l1|k1|j1|i1|h1|g1|f1|e1|d1|c1|b1|a1| which should be used for all modes.
// NO WARRANTY --- SEE STATEMENT IN TOP OF FILE (C) Ericsson AB 2005-2013. All Rights Reserved.
etcpak_force_inline uint32_t indexConversion(uint32_t pixelIndices)
{
  uint32_t correctIndices = 0;
  std::array<std::array<uint32_t, 4>, 4> LSB;
  std::array<std::array<uint32_t, 4>, 4> MSB;
  uint8_t shift = 0;
  for(int y = 3; y >= 0; y--)
  {
    for(int x = 3; x >= 0; x--)
    {
      LSB[x][y] = (pixelIndices >> shift) & 1u;
      shift++;
      MSB[x][y] = (pixelIndices >> shift) & 1u;
      shift++;
    }
  }
  shift = 0;
  for(int x = 0; x < 4; x++)
  {
    for(int y = 0; y < 4; y++)
    {
      correctIndices |= (LSB[x][y] << shift);
      correctIndices |= (MSB[x][y] << (16u + shift));
      shift++;
    }
  }
  return correctIndices;
}

// calculates quantized colors for T or H modes
std::array<BgrVec, 2> compressColor(const std::array<BgrVec, 2>& currColorBgr, bool t_mode)
{
  std::array<BgrVec, 2> result;
  if(t_mode)
  {
    result[0].r = std::min(15 * (currColorBgr[0].r + 8) / 255, 15);
    result[0].g = std::min(15 * (currColorBgr[0].g + 8) / 255, 15);
    result[0].b = std::min(15 * (currColorBgr[0].b + 8) / 255, 15);
  }
  else // clamped to [1,14] to get a wider range
  {
    result[0].r = std::clamp(15 * (currColorBgr[0].r + 8) / 255, 1, 14);
    result[0].g = std::clamp(15 * (currColorBgr[0].g + 8) / 255, 1, 14);
    result[0].b = std::clamp(15 * (currColorBgr[0].b + 8) / 255, 1, 14);
  }

  // clamped to [1,14] to get a wider range
  result[1].r = std::clamp(15 * (currColorBgr[1].r + 8) / 255, 1, 14);
  result[1].g = std::clamp(15 * (currColorBgr[1].g + 8) / 255, 1, 14);
  result[1].b = std::clamp(15 * (currColorBgr[1].b + 8) / 255, 1, 14);

  return result;
}

// three decoding functions come from ETCPACK v2.74 and are slightly changed.
etcpak_force_inline std::array<BgrVec, 2> decompressColor(const std::array<BgrVec, 2>& colorsBGR444)
{
  // The color should be retrieved as:
  //
  // c = round(255/(r_bits^2-1))*comp_color
  //
  // This is similar to bit replication
  //
  // Note -- this code only work for bit replication from 4 bits and up --- 3 bits needs
  // two copy operations.
  return std::array<BgrVec, 2>{{
    (colorsBGR444[0] << 4) | colorsBGR444[0],
    (colorsBGR444[1] << 4) | colorsBGR444[1],
  }};
}

std::array<BgrVec, 4> calculatePaintColors58H(uint8_t d, const std::array<BgrVec, 2>& bgr)
{
  return {
    // C1
    bgr[0] + tableTH[d],
    // C2
    bgr[0] - tableTH[d],
    // C3
    bgr[1] + tableTH[d],
    bgr[1] - tableTH[d],
  };
}

// calculates the paint colors from the block colors
// using a distance d and one of the H- or T-patterns.
std::array<BgrVec, 4> calculatePaintColors59T(uint8_t d, const std::array<BgrVec, 2>& bgr)
{
  //////////////////////////////////////////////
  //
  //		C3      C1		C4----C1---C2
  //		|		|			  |
  //		|		|			  |
  //		|-------|			  |
  //		|		|			  |
  //		|		|			  |
  //		C4      C2			  C3
  //
  //////////////////////////////////////////////

  return {
    // C3
    bgr[0],
    // C2
    bgr[1] + tableTH[d],
    // C1
    bgr[1],
    // C4
    bgr[1] - tableTH[d],
  };
}

using v4i = std::array<uint16_t, 4>;

etcpak_force_inline std::array<v4i, 8> average(const BgraBlockImm& bgra)
{
  auto d0 = bgra[0];
  auto d1 = bgra[1];
  auto d2 = bgra[2];
  auto d3 = bgra[3];

  auto sum0 = d0.lowToIVec16() + d1.lowToIVec16();
  auto sum1 = d0.highToIVec16() + d1.highToIVec16();
  auto sum2 = d2.lowToIVec16() + d3.lowToIVec16();
  auto sum3 = d2.highToIVec16() + d3.highToIVec16();

  auto b0 = sum0.lowToIVec32() + sum0.highToIVec32();
  auto b1 = sum1.lowToIVec32() + sum1.highToIVec32();
  auto b2 = sum2.lowToIVec32() + sum2.highToIVec32();
  auto b3 = sum3.lowToIVec32() + sum3.highToIVec32();

  auto a0 = (b2 + b3 + IVec{4}) >> 3;
  auto a1 = (b0 + b1 + IVec{4}) >> 3;
  auto a2 = (b1 + b3 + IVec{4}) >> 3;
  auto a3 = (b0 + b2 + IVec{4}) >> 3;

  std::array<v4i, 8> a;
  a0.shuffled<3, 0, 1, 2>().toIVec16U(a1.shuffled<3, 0, 1, 2>()).storeu(reinterpret_cast<__m128i*>(&a[0]));
  a2.shuffled<3, 0, 1, 2>().toIVec16U(a3.shuffled<3, 0, 1, 2>()).storeu(reinterpret_cast<__m128i*>(&a[2]));
  return a;
}

etcpak_force_inline std::array<std::array<uint32_t, 4>, 4> calcErrorBlock(const BgraBlockImm& bgra)
{
  auto dm0 = bgra[0] & 0x00FFFFFF;
  auto dm1 = bgra[1] & 0x00FFFFFF;
  auto dm2 = bgra[2] & 0x00FFFFFF;
  auto dm3 = bgra[3] & 0x00FFFFFF;

  auto sum0 = dm0.lowToIVec16() + dm1.lowToIVec16();
  auto sum1 = dm0.highToIVec16() + dm1.highToIVec16();
  auto sum2 = dm2.lowToIVec16() + dm3.lowToIVec16();
  auto sum3 = dm2.highToIVec16() + dm3.highToIVec16();

  auto b0 = sum0.lowToIVec32() + sum0.highToIVec32();
  auto b1 = sum1.lowToIVec32() + sum1.highToIVec32();
  auto b2 = sum2.lowToIVec32() + sum2.highToIVec32();
  auto b3 = sum3.lowToIVec32() + sum3.highToIVec32();

  std::array<std::array<uint32_t, 4>, 4> err;
  (b2 + b3).storeu(reinterpret_cast<__m128i*>(&err[0]));
  (b0 + b1).storeu(reinterpret_cast<__m128i*>(&err[1]));
  (b1 + b3).storeu(reinterpret_cast<__m128i*>(&err[2]));
  (b0 + b2).storeu(reinterpret_cast<__m128i*>(&err[3]));
  return err;
}

etcpak_force_inline uint32_t calcError(const std::array<uint32_t, 4>& block, const v4i& average)
{
  uint32_t err = 0x3FFFFFFF; // Big value to prevent negative values, but small enough to prevent overflow
  err -= block[0] * 2 * average[2];
  err -= block[1] * 2 * average[1];
  err -= block[2] * 2 * average[0];
  err += 8 * (sq(average[0]) + sq(average[1]) + sq(average[2]));
  return err;
}

etcpak_force_inline void processAverages(std::array<v4i, 8>& a)
{
  for(size_t i = 0; i < 2; i++)
  {
    auto d = IVec16{reinterpret_cast<const __m128i*>(a[i * 2].data())};

    auto t = d * IVec16{31} + IVec16{128};

    auto c = (t + (t >> 8)) >> 8;

    auto c1 = c.shuffled32<3, 2, 3, 2>();
    auto diff = c - c1;
    diff = diff.max(IVec16{-4});
    diff = diff.min(IVec16{3});

    auto co = c1 + diff;

    c = co.blended<0xF0>(c);

    auto a0 = (c << 3) | (c >> 2);

    a0.storeu(reinterpret_cast<__m128i*>(a[4 + i * 2].data()));
  }

  for(size_t i = 0; i < 2; i++)
  {
    auto d = IVec16{reinterpret_cast<const __m128i*>(a[i * 2].data())};

    auto t0 = d * IVec16{15} + IVec16{128};
    auto t1 = (t0 + (t0 >> 8)) >> 8;

    auto t2 = t1 | (t1 << 4);

    t2.storeu(reinterpret_cast<__m128i*>(a[i * 2].data()));
  }
}

etcpak_force_inline void encodeAverages(uint64_t& _d, const std::array<v4i, 8>& a, size_t idx)
{
  auto d = _d | (idx << 24u);
  const size_t base = idx << 1u;

  if((idx & 0x2u) == 0)
  {
    for(size_t i = 0; i < 3; i++)
    {
      d |= uint64_t(a[base + 0][i] >> 4u) << (i * 8);
      d |= uint64_t(a[base + 1][i] >> 4u) << (i * 8 + 4);
    }
  }
  else
  {
    for(size_t i = 0; i < 3; i++)
    {
      d |= uint64_t(a[base + 1][i] & 0xF8u) << (i * 8);
      uint32_t c = ((a[base + 0][i] & 0xF8u) - (a[base + 1][i] & 0xF8u)) >> 3u;
      c &= 7u;
      d |= ((uint64_t)c) << (i * 8);
    }
  }
  _d = d;
}

etcpak_force_inline uint32_t checkSolid(const BgraBlockImm& bgra)
{
  auto d0 = bgra[0];
  auto d1 = bgra[1];
  auto d2 = bgra[2];
  auto d3 = bgra[3];

  auto c = d0.shuffled32<0, 0, 0, 0>();

  auto m = (d0 == c) & (d1 == c) & (d2 == c) & (d3 == c);

  if(!m.testC(IVec{-1}))
  {
    return 0;
  }
  std::array<uint8_t, sizeof(__m128i)> tmp;
  (bgra[0] & IVec8{uint8_t(0xf8)}).storeu(reinterpret_cast<__m128i*>(tmp.data()));
  return 0x02000000u | ((uint32_t)tmp[0] << 16u) | ((uint32_t)tmp[1] << 8u) | ((uint32_t)tmp[2]);
}

etcpak_force_inline std::array<v4i, 8> prepareAverages(const BgraBlockImm& bgra, std::array<uint32_t, 4>& err)
{
  auto a = average(bgra);
  processAverages(a);

  auto errblock = calcErrorBlock(bgra);

  for(int i = 0; i < 4; i++)
  {
    err[i / 2] += calcError(errblock[i], a[i]);
    err[2 + i / 2] += calcError(errblock[i], a[i + 4]);
  }

  return a;
}

// Non-reference implementation, but faster. Produces same results as the AVX2 version
etcpak_force_inline void findBestFit(std::array<std::array<uint32_t, 8>, 2>& terr,
                                     std::array<std::array<uint16_t, 8>, 16>& tsel,
                                     const std::array<v4i, 8>& a,
                                     const std::array<uint32_t, 16>& id,
                                     const BgraVecBlock& bgra)
{
  auto it = bgra.begin();
  for(size_t i = 0; i < 16; i++)
  {
    const auto bid = id[i];

    const uint8_t b = it->b;
    const uint8_t g = it->g;
    const uint8_t r = it->r;
    ++it;

    const int dr = a[bid][0] - r;
    const int dg = a[bid][1] - g;
    const int db = a[bid][2] - b;

    // The scaling values are divided by two and rounded, to allow the differences to be in the range of signed int16
    // This produces slightly different results, but is significant faster
    auto pixel = IVec16(dr * 38 + dg * 76 + db * 14);
    auto pix = pixel.abs();

    // Taking the absolute value is way faster. The values are only used to sort, so the result will be the same.
    // Since the selector table is symmetrical, we need to calculate the difference only for half of the entries.
    auto error0 = (pix - g_table128_SIMD[0]).abs();
    auto error1 = (pix - g_table128_SIMD[1]).abs();

    auto index = (error1 < error0) & IVec16{1};
    auto minError = error0.min(error1);

    // Exploiting symmetry of the selector table and use the sign bit
    // This produces slightly different results, but is needed to produce same results as AVX2 implementation
    auto indexBit = IVec16{_mm_andnot_si128((pixel >> 15).data, _mm_set1_epi8(-1))};
    auto minIndex = index | (indexBit + indexBit);

    // Squaring the minimum error to produce correct values when adding
    auto squareErrorLo = minError * minError;
    auto squareErrorHi = minError.mulHi(minError);

    auto squareErrorLow = squareErrorLo.lowToIVec32(squareErrorHi);
    auto squareErrorHigh = squareErrorLo.highToIVec32(squareErrorHi);

    auto& ter = terr[bid % 2];
    squareErrorLow = squareErrorLow + IVec{reinterpret_cast<const __m128i*>(ter.data()) + 0};
    squareErrorLow.storeu(reinterpret_cast<__m128i*>(ter.data()) + 0);
    squareErrorHigh = squareErrorHigh + IVec{reinterpret_cast<const __m128i*>(ter.data()) + 1};
    squareErrorHigh.storeu(reinterpret_cast<__m128i*>(ter.data()) + 1);

    minIndex.storeu(reinterpret_cast<__m128i*>(tsel[i].data()));
  }
}

etcpak_force_inline uint8_t convert6(float f)
{
  const int i = (std::min(std::max(static_cast<int>(f), 0), 1023) - 15) >> 1;
  return (i + 11 - ((i + 11) >> 7) - ((i + 4) >> 7)) >> 3;
}

etcpak_force_inline uint8_t convert7(float f)
{
  const int i = (std::min(std::max(static_cast<int>(f), 0), 1023) - 15) >> 1;
  return (i + 9 - ((i + 9) >> 8) - ((i + 6) >> 8)) >> 2;
}

etcpak_force_inline std::pair<uint64_t, uint64_t>
  planar(const BgraVecBlock& bgra, const uint8_t mode, bool useHeuristics)
{
  int32_t r = 0;
  int32_t g = 0;
  int32_t b = 0;

  for(int i = 0; i < 16; ++i)
  {
    b += bgra[i].b;
    g += bgra[i].g;
    r += bgra[i].r;
  }

  int32_t difRyz = 0;
  int32_t difGyz = 0;
  int32_t difByz = 0;
  int32_t difRxz = 0;
  int32_t difGxz = 0;
  int32_t difBxz = 0;

  static constexpr std::array<int32_t, 4> scaling{{-255, -85, 85, 255}};

  for(int i = 0; i < 16; ++i)
  {
    const int32_t difB = (gsl::narrow_cast<int>(bgra[i].b) << 4u) - b;
    const int32_t difG = (gsl::narrow_cast<int>(bgra[i].g) << 4u) - g;
    const int32_t difR = (gsl::narrow_cast<int>(bgra[i].r) << 4u) - r;

    difRyz += difR * scaling[i % 4];
    difGyz += difG * scaling[i % 4];
    difByz += difB * scaling[i % 4];

    difRxz += difR * scaling[i / 4];
    difGxz += difG * scaling[i / 4];
    difBxz += difB * scaling[i / 4];
  }

  static const float scale = -4.0f / ((255 * 255 * 8.0f + 85 * 85 * 8.0f) * 16.0f);

  const float aR = gsl::narrow_cast<float>(difRxz) * scale;
  const float aG = gsl::narrow_cast<float>(difGxz) * scale;
  const float aB = gsl::narrow_cast<float>(difBxz) * scale;

  const float bR = gsl::narrow_cast<float>(difRyz) * scale;
  const float bG = gsl::narrow_cast<float>(difGyz) * scale;
  const float bB = gsl::narrow_cast<float>(difByz) * scale;

  const float dR = gsl::narrow_cast<float>(r) * (4.0f / 16.0f);
  const float dG = gsl::narrow_cast<float>(g) * (4.0f / 16.0f);
  const float dB = gsl::narrow_cast<float>(b) * (4.0f / 16.0f);

  // calculating the three colors RGBO, RGBH, and RGBV.  RGB = df - af * x - bf * y;
  const float cofR = std::fma(aR, 255.0f, std::fma(bR, 255.0f, dR));
  const float cofG = std::fma(aG, 255.0f, std::fma(bG, 255.0f, dG));
  const float cofB = std::fma(aB, 255.0f, std::fma(bB, 255.0f, dB));
  const float chfR = std::fma(aR, -425.0f, std::fma(bR, 255.0f, dR));
  const float chfG = std::fma(aG, -425.0f, std::fma(bG, 255.0f, dG));
  const float chfB = std::fma(aB, -425.0f, std::fma(bB, 255.0f, dB));
  const float cvfR = std::fma(aR, 255.0f, std::fma(bR, -425.0f, dR));
  const float cvfG = std::fma(aG, 255.0f, std::fma(bG, -425.0f, dG));
  const float cvfB = std::fma(aB, 255.0f, std::fma(bB, -425.0f, dB));

  // convert to r6g7b6
  const uint32_t coR = convert6(cofR);
  const uint32_t coG = convert7(cofG);
  const uint32_t coB = convert6(cofB);
  const uint32_t chR = convert6(chfR);
  const uint32_t chG = convert7(chfG);
  const uint32_t chB = convert6(chfB);
  const uint32_t cvR = convert6(cvfR);
  const uint32_t cvG = convert7(cvfG);
  const uint32_t cvB = convert6(cvfB);

  // Error calculation
  uint64_t error = 0;
  if(ModePlanar != mode && useHeuristics)
  {
    auto ro0 = coR;
    auto go0 = coG;
    auto bo0 = coB;
    auto ro1 = (ro0 >> 4u) | (ro0 << 2u);
    auto go1 = (go0 >> 6u) | (go0 << 1u);
    auto bo1 = (bo0 >> 4u) | (bo0 << 2u);
    auto ro2 = (ro1 << 2u) + 2u;
    auto go2 = (go1 << 2u) + 2u;
    auto bo2 = (bo1 << 2u) + 2u;

    auto rh0 = chR;
    auto gh0 = chG;
    auto bh0 = chB;
    auto rh1 = (rh0 >> 4u) | (rh0 << 2u);
    auto gh1 = (gh0 >> 6u) | (gh0 << 1u);
    auto bh1 = (bh0 >> 4u) | (bh0 << 2u);

    auto rh2 = rh1 - ro1;
    auto gh2 = gh1 - go1;
    auto bh2 = bh1 - bo1;

    auto rv0 = cvR;
    auto gv0 = cvG;
    auto bv0 = cvB;
    auto rv1 = (rv0 >> 4u) | (rv0 << 2u);
    auto gv1 = (gv0 >> 6u) | (gv0 << 1u);
    auto bv1 = (bv0 >> 4u) | (bv0 << 2u);

    auto rv2 = rv1 - ro1;
    auto gv2 = gv1 - go1;
    auto bv2 = bv1 - bo1;
    for(uint32_t i = 0; i < 16; ++i)
    {
      const int32_t cR = clampu8((rh2 * (i / 4u) + rv2 * (i % 4u) + ro2) >> 2u);
      const int32_t cG = clampu8((gh2 * (i / 4u) + gv2 * (i % 4u) + go2) >> 2u);
      const int32_t cB = clampu8((bh2 * (i / 4u) + bv2 * (i % 4u) + bo2) >> 2u);

      const int32_t difB = static_cast<int>(bgra[i].b) - cB;
      const int32_t difG = static_cast<int>(bgra[i].g) - cG;
      const int32_t difR = static_cast<int>(bgra[i].r) - cR;

      const int32_t dif = difR * 38 + difG * 76 + difB * 14;

      error += dif * dif;
    }
  }

  /**/
  const uint32_t rgbv = cvB | (cvG << 6u) | (cvR << 13u);
  const uint32_t rgbh = chB | (chG << 6u) | (chR << 13u);
  const uint32_t hi = rgbv | ((rgbh & 0x1FFFu) << 19u);
  uint32_t lo = (chR & 0x1u) | 0x2u | ((chR << 1u) & 0x7Cu);
  lo |= ((coB & 0x07u) << 7u) | ((coB & 0x18u) << 8u) | ((coB & 0x20u) << 11u);
  lo |= ((coG & 0x3Fu) << 17u) | ((coG & 0x40u) << 18u);
  lo |= coR << 25u;

  const auto idx = (coR & 0x20u) | ((coG & 0x20u) >> 1u) | ((coB & 0x1Eu) >> 1u);

  lo |= g_flags[idx];

  uint64_t result = static_cast<uint32_t>(_bswap(lo));
  result |= static_cast<uint64_t>(static_cast<uint32_t>(_bswap(hi))) << 32u;

  return std::make_pair(result, error);
}

uint32_t calculateErrorTH(bool tMode,
                          const BgraVecBlock& bgra,
                          const std::array<BgrVec, 2>& colorsBGR444,
                          uint8_t& dist,
                          uint32_t& pixIndices,
                          uint8_t startDist)
{
  uint32_t blockErr = 0, bestBlockErr = MaxError;

  uint32_t pixColors;

  auto colors = decompressColor(colorsBGR444);

  // test distances
  for(uint8_t d = startDist; d < 8; ++d)
  {
    if(d >= 2 && dist == d - 2)
      break;

    blockErr = 0;
    pixColors = 0;

    const auto possibleColors{tMode ? calculatePaintColors59T(d, colors) : calculatePaintColors58H(d, colors)};

    for(size_t y = 0; y < 4; ++y)
    {
      for(size_t x = 0; x < 4; ++x)
      {
        uint32_t bestPixErr = MaxError;
        pixColors <<= 2u; // Make room for next value

        // Loop possible block colors
        for(uint8_t c = 0; c < 4; ++c)
        {
          const glm::ivec3 diff{
            bgra[x * 4 + y].r - possibleColors[c].r,
            bgra[x * 4 + y].g - possibleColors[c].g,
            bgra[x * 4 + y].b - possibleColors[c].b,
          };

          const uint32_t err = 38 * abs(diff.r) + 76 * abs(diff.g) + 14 * abs(diff.b);
          const uint32_t pixErr = err * err;

          // Choose best error
          if(pixErr < bestPixErr)
          {
            bestPixErr = pixErr;
            pixColors ^= (pixColors & 3u); // Reset the two first bits
            pixColors |= c;
          }
        }
        blockErr += bestPixErr;
      }
    }

    if(blockErr < bestBlockErr)
    {
      bestBlockErr = blockErr;
      dist = d;
      pixIndices = pixColors;
    }
  }

  return bestBlockErr;
}

// main T-/H-mode compression function
uint32_t compressBlockTH(const BgraVecBlock& bgra, Luma& l, uint32_t& compressed1, uint32_t& compressed2, bool& tMode)
{
  auto& luma = l.val;

  std::array<uint8_t, 16> pixIdx{{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}};

  // 1) sorts the pairs of (luma, pix_idx)
  insertionSort(luma, pixIdx);

  // 2) finds the min (left+right)
  uint8_t minSumRangeIdx = 0;
  uint16_t sum;
  static constexpr std::array<uint8_t, 15> diffBonus{{8, 4, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 4, 8}};

  uint16_t minSumRangeValue = luma[15] - luma[1] + diffBonus[0];

  {
    const auto temp = gsl::narrow_cast<int16_t>(luma[15] - luma[0]);
    for(uint8_t i = 1; i < 14; i++)
    {
      sum = temp - luma[i + 1] + luma[i] + diffBonus[i];
      if(minSumRangeValue > sum)
      {
        minSumRangeValue = sum;
        minSumRangeIdx = i;
      }
    }
  }

  sum = luma[14] - luma[0] + diffBonus[14];
  if(minSumRangeValue > sum)
  {
    minSumRangeValue = sum;
    minSumRangeIdx = 14;
  }
  uint8_t lRange, rRange;

  lRange = luma[minSumRangeIdx] - luma[0];
  rRange = luma[15] - luma[minSumRangeIdx + 1];

  // 3) sets a proper mode
  bool swap = false;
  if(lRange >= rRange)
  {
    if(lRange >= rRange * 2)
    {
      swap = true;
      tMode = true;
    }
  }
  else
  {
    if(lRange * 2 <= rRange)
      tMode = true;
  }
  // 4) calculates the two base colors
  std::array<uint8_t, 4> rangeIdx{{pixIdx[0], pixIdx[minSumRangeIdx], pixIdx[minSumRangeIdx + 1], pixIdx[15]}};

  std::array<uint16_t, 4> r, g, b;
  for(uint8_t i = 0; i < 4; ++i)
  {
    const uint8_t idx = rangeIdx[i];
    b[i] = bgra[idx].b;
    g[i] = bgra[idx].g;
    r[i] = bgra[idx].r;
  }

  std::array<BgrVec, 2> midBgr;
  if(swap)
  {
    midBgr[1].b = (b[0] + b[1]) / 2;
    midBgr[1].g = (g[0] + g[1]) / 2;
    midBgr[1].r = (r[0] + r[1]) / 2;

    glm::u16vec3 sumBgr{0, 0, 0};
    for(uint8_t i = minSumRangeIdx + 1; i < 16; i++)
    {
      const auto idx = pixIdx[i];
      sumBgr.b += bgra[idx].b;
      sumBgr.g += bgra[idx].g;
      sumBgr.r += bgra[idx].r;
    }
    const uint8_t temp = 15 - minSumRangeIdx;
    midBgr[0].b = sumBgr.b / temp;
    midBgr[0].g = sumBgr.g / temp;
    midBgr[0].r = sumBgr.r / temp;
  }
  else
  {
    midBgr[0].b = (b[0] + b[1]) / 2;
    midBgr[0].g = (g[0] + g[1]) / 2;
    midBgr[0].r = (r[0] + r[1]) / 2;
    if(tMode)
    {
      glm::u16vec3 sumBgr{0, 0, 0};
      for(uint8_t i = minSumRangeIdx + 1; i < 16; i++)
      {
        const auto idx = pixIdx[i];
        sumBgr.b += bgra[idx].b;
        sumBgr.g += bgra[idx].g;
        sumBgr.r += bgra[idx].r;
      }
      const uint8_t temp = 15 - minSumRangeIdx;
      midBgr[1].b = sumBgr.b / temp;
      midBgr[1].g = sumBgr.g / temp;
      midBgr[1].r = sumBgr.r / temp;
    }
    else
    {
      midBgr[1].b = (b[2] + b[3]) / 2;
      midBgr[1].g = (g[2] + g[3]) / 2;
      midBgr[1].r = (r[2] + r[3]) / 2;
    }
  }

  // 5) sets the start distance index
  uint32_t avgDist;
  if(tMode)
  {
    if(swap)
    {
      avgDist = (b[1] - b[0] + g[1] - g[0] + r[1] - r[0]) / 6;
    }
    else
    {
      avgDist = (b[3] - b[2] + g[3] - g[2] + r[3] - r[2]) / 6;
    }
  }
  else
  {
    avgDist = (b[1] - b[0] + g[1] - g[0] + r[1] - r[0] + b[3] - b[2] + g[3] - g[2] + r[3] - r[2]) / 12;
  }

  uint32_t startDistCandidate;
  if(avgDist <= 16)
  {
    startDistCandidate = 0;
  }
  else if(avgDist <= 23)
  {
    startDistCandidate = 1;
  }
  else if(avgDist <= 32)
  {
    startDistCandidate = 2;
  }
  else if(avgDist <= 41)
  {
    startDistCandidate = 3;
  }
  else
  {
    startDistCandidate = 4;
  }

  uint32_t bestErr = MaxError;
  uint32_t bestPixIndices;
  uint8_t bestDist = 10;
  auto colorsRGB444 = compressColor(midBgr, tMode);
  compressed1 = 0;

  // 6) finds the best candidate with the lowest error
  // Scalar ver
  bestErr = calculateErrorTH(tMode, bgra, colorsRGB444, bestDist, bestPixIndices, startDistCandidate);

  // 7) outputs the final T or H block
  if(tMode)
  {
    // Put the compress params into the compression block
    compressed1 |= (colorsRGB444[0].r & 0xfu) << 23u;
    compressed1 |= (colorsRGB444[0].g & 0xfu) << 19u;
    compressed1 |= (colorsRGB444[0].b) << 15u;
    compressed1 |= (colorsRGB444[1].r) << 11u;
    compressed1 |= (colorsRGB444[1].g) << 7u;
    compressed1 |= (colorsRGB444[1].b) << 3u;
    compressed1 |= bestDist & 0x7u;
  }
  else
  {
    std::array<int, 2> bestRGB444ColPacked{{
      (colorsRGB444[0].r << 8u) + (colorsRGB444[0].g << 4u) + colorsRGB444[0].b,
      (colorsRGB444[1].r << 8u) + (colorsRGB444[1].g << 4u) + colorsRGB444[1].b,
    }};
    if((bestRGB444ColPacked[0] >= bestRGB444ColPacked[1]) ^ ((bestDist & 1u) == 1))
    {
      std::swap(colorsRGB444[0], colorsRGB444[1]);
      // Reshuffle pixel indices to to exchange C1 with C3, and C2 with C4
      bestPixIndices = (0x55555555u & bestPixIndices) | (0xaaaaaaaau & ~bestPixIndices);
    }

    // Put the compress params into the compression block
    compressed1 |= (colorsRGB444[0].r & 0xfu) << 22u;
    compressed1 |= (colorsRGB444[0].g & 0xfu) << 18u;
    compressed1 |= (colorsRGB444[0].b & 0xfu) << 14u;
    compressed1 |= (colorsRGB444[1].r & 0xfu) << 10u;
    compressed1 |= (colorsRGB444[1].g & 0xfu) << 6u;
    compressed1 |= (colorsRGB444[1].b & 0xfu) << 2u;
    compressed1 |= (bestDist >> 1u) & 0x3u;
  }

  bestPixIndices = indexConversion(bestPixIndices);
  compressed2 = (bestPixIndices & ((2u << 31u) - 1u));

  return bestErr;
}

etcpak_force_inline uint64_t encodeSelectors(uint64_t d,
                                             const std::array<std::array<uint32_t, 8>, 2>& terr,
                                             const std::array<std::array<uint16_t, 8>, 16>& tsel,
                                             const std::array<uint32_t, 16>& id,
                                             const uint64_t value,
                                             const uint64_t error)
{
  std::array<size_t, 2> tidx{{
    getLeastError(terr[0]),
    getLeastError(terr[1]),
  }};

  if((terr[0][tidx[0]] + terr[1][tidx[1]]) >= error)
  {
    return value;
  }

  d |= tidx[0] << 26u;
  d |= tidx[1] << 29u;
  for(size_t i = 0; i < 16; i++)
  {
    const uint64_t t = tsel[i][tidx[id[i] % 2]];
    d |= (t & 0x1u) << (i + 32u);
    d |= (t & 0x2u) << (i + 47u);
  }

  return fixByteOrder(d);
}

// During search it is not convenient to store the bits the way they are stored in the
// file format. Hence, after search, it is converted to this format.
// NO WARRANTY --- SEE STATEMENT IN TOP OF FILE (C) Ericsson AB 2005-2013. All Rights Reserved.
inline void stuff59bits(uint32_t thumbT59W1, uint32_t thumbT59W2, uint32_t& thumbTW1, uint32_t& thumbTW2)
{
  // Put bits in twotimer configuration for 59 (red overflows)
  //
  // Go from this bit layout:
  //
  //     |63 62 61 60 59|58 57 56 55|54 53 52 51|50 49 48 47|46 45 44 43|42 41 40 39|38 37 36 35|34 33 32|
  //     |----empty-----|---red 0---|--green 0--|--blue 0---|---red 1---|--green 1--|--blue 1---|--dist--|
  //
  //     |31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00|
  //     |----------------------------------------index bits---------------------------------------------|
  //
  //
  //  To this:
  //
  //      63 62 61 60 59 58 57 56 55 54 53 52 51 50 49 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34 33 32
  //      -----------------------------------------------------------------------------------------------
  //     |// // //|R0a  |//|R0b  |G0         |B0         |R1         |G1         |B1          |da  |df|db|
  //      -----------------------------------------------------------------------------------------------
  //
  //     |31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00|
  //     |----------------------------------------index bits---------------------------------------------|
  //
  //      63 62 61 60 59 58 57 56 55 54 53 52 51 50 49 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34 33 32
  //      -----------------------------------------------------------------------------------------------
  //     | base col1    | dcol 2 | base col1    | dcol 2 | base col 1   | dcol 2 | table  | table  |df|fp|
  //     | R1' (5 bits) | dR2    | G1' (5 bits) | dG2    | B1' (5 bits) | dB2    | cw 1   | cw 2   |bt|bt|
  //      ------------------------------------------------------------------------------------------------

  uint8_t R0a;
  uint8_t bit, a, b, c, d, bits;

  R0a = (thumbT59W1 >> 25u) & 0x3u;

  // Fix middle part
  thumbTW1 = thumbT59W1 << 1u;
  // Fix R0a (top two bits of R0)
  thumbTW1 = (thumbTW1 & ~(0x3u << 27u)) | ((R0a & 0x3u) << 27u);
  // Fix db (lowest bit of d)
  thumbTW1 = (thumbTW1 & ~0x1u) | (thumbT59W1 & 0x1u);

  // Make sure that red overflows:
  a = (thumbTW1 >> 28u) & 0x1u;
  b = (thumbTW1 >> 27u) & 0x1u;
  c = (thumbTW1 >> 25u) & 0x1u;
  d = (thumbTW1 >> 24u) & 0x1u;

  // The following bit abcd bit sequences should be padded with ones: 0111, 1010, 1011, 1101, 1110, 1111
  // The following logical expression checks for the presence of any of those:
  bit = (a & c) | (!a & b & c & d) | (a & b & !c & d);
  bits = 0xf * bit;
  thumbTW1 = (thumbTW1 & ~(0x7u << 29u)) | (bits & 0x7u) << 29u;
  thumbTW1 = (thumbTW1 & ~(0x1u << 26u)) | (~bit & 0x1u) << 26u;

  // Set diffbit
  thumbTW1 = (thumbTW1 & ~0x2u) | 0x2u;
  thumbTW2 = thumbT59W2;
}

// During search it is not convenient to store the bits the way they are stored in the
// file format. Hence, after search, it is converted to this format.
// NO WARRANTY --- SEE STATEMENT IN TOP OF FILE (C) Ericsson AB 2005-2013. All Rights Reserved.
inline void stuff58bits(uint32_t thumbH58W1, uint32_t thumbH58W2, uint32_t& thumbHW1, uint32_t& thumbHW2)
{
  // Put bits in twotimer configuration for 58 (red doesn't overflow, green does)
  //
  // Go from this bit layout:
  //
  //
  //     |63 62 61 60 59 58|57 56 55 54|53 52 51 50|49 48 47 46|45 44 43 42|41 40 39 38|37 36 35 34|33 32|
  //     |-------empty-----|---red 0---|--green 0--|--blue 0---|---red 1---|--green 1--|--blue 1---|d2 d1|
  //
  //     |31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00|
  //     |---------------------------------------index bits----------------------------------------------|
  //
  //  To this:
  //
  //      63 62 61 60 59 58 57 56 55 54 53 52 51 50 49 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34 33 32
  //      -----------------------------------------------------------------------------------------------
  //     |//|R0         |G0      |// // //|G0|B0|//|B0b     |R1         |G1         |B0         |d2|df|d1|
  //      -----------------------------------------------------------------------------------------------
  //
  //     |31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00|
  //     |---------------------------------------index bits----------------------------------------------|
  //
  //      63 62 61 60 59 58 57 56 55 54 53 52 51 50 49 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34 33 32
  //      -----------------------------------------------------------------------------------------------
  //     | base col1    | dcol 2 | base col1    | dcol 2 | base col 1   | dcol 2 | table  | table  |df|fp|
  //     | R1' (5 bits) | dR2    | G1' (5 bits) | dG2    | B1' (5 bits) | dB2    | cw 1   | cw 2   |bt|bt|
  //      -----------------------------------------------------------------------------------------------
  //
  //
  // Thus, what we are really doing is going from this bit layout:
  //
  //
  //     |63 62 61 60 59 58|57 56 55 54 53 52 51|50 49|48 47 46 45 44 43 42 41 40 39 38 37 36 35 34 33|32   |
  //     |-------empty-----|part0---------------|part1|part2------------------------------------------|part3|
  //
  //  To this:
  //
  //      63 62 61 60 59 58 57 56 55 54 53 52 51 50 49 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34 33 32
  //      --------------------------------------------------------------------------------------------------|
  //     |//|part0               |// // //|part1|//|part2                                          |df|part3|
  //      --------------------------------------------------------------------------------------------------|

  uint32_t part0, part1, part2, part3;
  uint8_t bit, a, b, c, d, bits;

  // move parts
  part0 = (thumbH58W1 >> 19u) & 0x7fu;
  part1 = (thumbH58W1 >> 17u) & 0x3u;
  part2 = (thumbH58W1 >> 1u) & 0xffffu;
  part3 = thumbH58W1 & 0x1u;
  thumbHW1 = 0;
  thumbHW1 = (thumbHW1 & ~(0x7fu << 24u)) | ((part0 & 0x7fu) << 24u);
  thumbHW1 = (thumbHW1 & ~(0x3u << 19u)) | ((part1 & 0x3u) << 19u);
  thumbHW1 = (thumbHW1 & ~(0xffffu << 2u)) | ((part2 & 0xffffu) << 2u);
  thumbHW1 = (thumbHW1 & ~0x1u) | (part3 & 0x1u);

  // Make sure that red does not overflow:
  bit = (thumbHW1 >> 30u) & 0x1u;
  thumbHW1 = (thumbHW1 & ~(0x1u << 31u)) | ((~bit & 0x1u) << 31u);

  // Make sure that green overflows:
  a = (thumbHW1 >> 20u) & 0x1u;
  b = (thumbHW1 >> 19u) & 0x1u;
  c = (thumbHW1 >> 17u) & 0x1u;
  d = (thumbHW1 >> 16u) & 0x1u;
  // The following bit abcd bit sequences should be padded with ones: 0111, 1010, 1011, 1101, 1110, 1111
  // The following logical expression checks for the presence of any of those:
  bit = (a & c) | (!a & b & c & d) | (a & b & !c & d);
  bits = 0xf * bit;
  thumbHW1 = (thumbHW1 & ~(0x7u << 21u)) | ((bits & 0x7u) << 21u);
  thumbHW1 = (thumbHW1 & ~(0x1u << 18u)) | ((~bit & 0x1u) << 18u);

  // Set diffbit
  thumbHW1 = thumbHW1 | 0x2u;
  thumbHW2 = thumbH58W2;
}

etcpak_force_inline void calculateLuma(const BgraVecBlock& bgra, Luma& luma)
{
  for(uint8_t i = 0; i < 16; ++i)
  {
    luma.val[i] = (bgra[i].r * 76 + bgra[i].g * 150 + bgra[i].b * 28) / 254; // luma calculation
    if(luma.min > luma.val[i])
    {
      luma.min = luma.val[i];
      luma.minIdx = i;
    }
    if(luma.max < luma.val[i])
    {
      luma.max = luma.val[i];
      luma.maxIdx = i;
    }
  }
}

etcpak_force_inline uint8_t selectModeETC2(const Luma& luma)
{
  const float lumaRange = gsl::narrow_cast<float>(luma.max - luma.min) / 255.0f;
  // filters a very-low-contrast block
  if(lumaRange <= ecmdThreshold[0])
  {
    return ModePlanar;
  }
  // checks whether a pair of the corner pixels in a block has the min/max luma values;
  // if so, the ETC2 planar mode is enabled, and otherwise, the ETC1 mode is enabled
  else if(lumaRange <= ecmdThreshold[1])
  {
    // check whether a pair of the corner pixels in a block has the min/max luma values;
    // if so, the ETC2 planar mode is enabled.
    if((luma.minIdx == 0 && luma.maxIdx == 15) || (luma.minIdx == 15 && luma.maxIdx == 0)
       || (luma.minIdx == 3 && luma.maxIdx == 12) || (luma.minIdx == 12 && luma.maxIdx == 3))
    {
      return ModePlanar;
    }
  }
  // filters a high-contrast block for checking both ETC1 mode and the ETC2 T/H mode
  else if(lumaRange >= ecmdThreshold[2])
  {
    return ModeTH;
  }
  return ModeUndecided;
}

etcpak_force_inline uint64_t processBGR_ETC2(const BgraVecBlock& bgra, const BgraBlockImm& immRgba, bool useHeuristics)
{
  uint64_t d = checkSolid(immRgba);
  if(d != 0)
    return d;

  uint8_t mode = ModeUndecided;
  Luma luma;
  if(useHeuristics)
  {
    calculateLuma(bgra, luma);
    mode = selectModeETC2(luma);
  }
  auto [encoded, error] = planar(bgra, mode, useHeuristics);
  if(error == 0)
    return encoded;

  std::array<uint32_t, 4> err{};
  auto a = prepareAverages(immRgba, err);
  const size_t idx = getLeastError(err);
  encodeAverages(d, a, idx);

  std::array<std::array<uint32_t, 8>, 2> terr{};
  std::array<std::array<uint16_t, 8>, 16> tsel;
  const auto& id = g_id[idx];
  findBestFit(terr, tsel, a, id, bgra);

  if(useHeuristics)
  {
    if(mode == ModeTH)
    {
      std::array<uint32_t, 4> compressed{{0, 0, 0, 0}};
      bool tMode = false;

      error = compressBlockTH(bgra, luma, compressed[0], compressed[1], tMode);
      if(tMode)
      {
        stuff59bits(compressed[0], compressed[1], compressed[2], compressed[3]);
      }
      else
      {
        stuff58bits(compressed[0], compressed[1], compressed[2], compressed[3]);
      }

      encoded = (uint32_t)_bswap(compressed[2]);
      encoded |= static_cast<uint64_t>(_bswap(compressed[3])) << 32u;
    }
    else
    {
      encoded = 0;
      error = MaxError;
    }
  }

  return encodeSelectors(d, terr, tsel, id, encoded, error);
}

template<int K>
etcpak_force_inline IVec16 widen(const IVec16& src)
{
  static_assert(K >= 0 && K <= 7, "Index out of range");

  constexpr uint8_t s1 = K % 4u;
  constexpr uint8_t s2 = (K / 4u) * 2u;

  const auto tmp = _mm_shufflelo_epi16(src.data, _MM_SHUFFLE(s1, s1, s1, s1));
  return IVec16{_mm_shuffle_epi32(tmp, _MM_SHUFFLE(s2, s2, s2, s2))};
}

constexpr etcpak_force_inline int getMulSel(int sel)
{
  switch(sel)
  {
  case 0:
    return 0;
  case 1:
  case 2:
  case 3:
    return 1;
  case 4:
    return 2;
  case 5:
  case 6:
  case 7:
    return 3;
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
    return 4;
  case 14:
  case 15:
    return 5;
  }
  assert(false);
}

etcpak_force_inline uint64_t processAlpha_ETC2(const IVec8& alphas)
{
  // Check solid
  if((alphas == alphas.shuffled(IVec{0})).testC(IVec{-1}))
  {
    std::array<uint8_t, sizeof(__m128i)> tmp;
    alphas.storeu(reinterpret_cast<__m128i*>(tmp.data()));
    return tmp[0];
  }

  // Calculate min, max
  auto s1 = alphas.shuffled32<2, 3, 0, 1>();
  auto max1 = alphas.umax(s1);
  auto min1 = alphas.umin(s1);
  auto smax2 = max1.shuffled32<0, 0, 2, 2>();
  auto smin2 = min1.shuffled32<0, 0, 2, 2>();
  auto max2 = max1.umax(smax2);
  auto min2 = min1.umin(smin2);
  auto smax3 = max2.alignR<2>(max2);
  auto smin3 = min2.alignR<2>(min2);
  auto max3 = max2.umax(smax3);
  auto min3 = min2.umin(smin3);
  auto smax4 = max3.alignR<1>(max3);
  auto smin4 = min3.alignR<1>(min3);
  auto max = max3.umax(smax4);
  auto min = min3.umin(smin4);
  auto max16 = max.lowToIVec16();
  auto min16 = min.highToIVec16();

  // src range, mid
  auto srcRange = max16 - min16;
  auto srcRangeHalf = srcRange >> 1;
  auto srcMid = min16 + srcRangeHalf;

  // multiplier
  auto mul1 = srcRange.mulHi(g_alphaRange_SIMD);
  auto mul = mul1 + IVec16{1};

  // wide source
  std::array<IVec16, 2> s16{{
    alphas.lowToIVec16(),
    alphas.shuffled32<3, 2, 3, 2>().lowToIVec16(),
  }};

  std::array<IVec16, 16> sr{{
    widen<0>(s16[0]),
    widen<1>(s16[0]),
    widen<2>(s16[0]),
    widen<3>(s16[0]),
    widen<4>(s16[0]),
    widen<5>(s16[0]),
    widen<6>(s16[0]),
    widen<7>(s16[0]),
    widen<0>(s16[1]),
    widen<1>(s16[1]),
    widen<2>(s16[1]),
    widen<3>(s16[1]),
    widen<4>(s16[1]),
    widen<5>(s16[1]),
    widen<6>(s16[1]),
    widen<7>(s16[1]),
  }};

  // wide multiplier
  std::array<IVec16, 16> rangeMul{{
    (srcMid + widen<0>(mul) * g_alpha_SIMD[0]).toIVec8U(srcMid + widen<0>(mul) * g_alpha_SIMD[0]).lowToIVec16(),
    (srcMid + widen<1>(mul) * g_alpha_SIMD[1]).toIVec8U(srcMid + widen<1>(mul) * g_alpha_SIMD[1]).lowToIVec16(),
    (srcMid + widen<1>(mul) * g_alpha_SIMD[2]).toIVec8U(srcMid + widen<1>(mul) * g_alpha_SIMD[2]).lowToIVec16(),
    (srcMid + widen<1>(mul) * g_alpha_SIMD[3]).toIVec8U(srcMid + widen<1>(mul) * g_alpha_SIMD[3]).lowToIVec16(),
    (srcMid + widen<2>(mul) * g_alpha_SIMD[4]).toIVec8U(srcMid + widen<2>(mul) * g_alpha_SIMD[4]).lowToIVec16(),
    (srcMid + widen<3>(mul) * g_alpha_SIMD[5]).toIVec8U(srcMid + widen<3>(mul) * g_alpha_SIMD[5]).lowToIVec16(),
    (srcMid + widen<3>(mul) * g_alpha_SIMD[6]).toIVec8U(srcMid + widen<3>(mul) * g_alpha_SIMD[6]).lowToIVec16(),
    (srcMid + widen<3>(mul) * g_alpha_SIMD[7]).toIVec8U(srcMid + widen<3>(mul) * g_alpha_SIMD[7]).lowToIVec16(),
    (srcMid + widen<4>(mul) * g_alpha_SIMD[8]).toIVec8U(srcMid + widen<4>(mul) * g_alpha_SIMD[8]).lowToIVec16(),
    (srcMid + widen<4>(mul) * g_alpha_SIMD[9]).toIVec8U(srcMid + widen<4>(mul) * g_alpha_SIMD[9]).lowToIVec16(),
    (srcMid + widen<4>(mul) * g_alpha_SIMD[10]).toIVec8U(srcMid + widen<4>(mul) * g_alpha_SIMD[10]).lowToIVec16(),
    (srcMid + widen<4>(mul) * g_alpha_SIMD[11]).toIVec8U(srcMid + widen<4>(mul) * g_alpha_SIMD[11]).lowToIVec16(),
    (srcMid + widen<4>(mul) * g_alpha_SIMD[12]).toIVec8U(srcMid + widen<4>(mul) * g_alpha_SIMD[12]).lowToIVec16(),
    (srcMid + widen<4>(mul) * g_alpha_SIMD[13]).toIVec8U(srcMid + widen<4>(mul) * g_alpha_SIMD[13]).lowToIVec16(),
    (srcMid + widen<5>(mul) * g_alpha_SIMD[14]).toIVec8U(srcMid + widen<5>(mul) * g_alpha_SIMD[14]).lowToIVec16(),
    (srcMid + widen<5>(mul) * g_alpha_SIMD[15]).toIVec8U(srcMid + widen<5>(mul) * g_alpha_SIMD[15]).lowToIVec16(),
  }};

  // find selector
  int err = std::numeric_limits<int>::max();
  int sel;
  for(int r = 0; r < 16; r++)
  {
    auto recVal16 = rangeMul[r];
    int rangeErr = 0;

    for(const auto& sri : sr)
    {
      auto err1 = sri - recVal16;
      auto err2 = err1 * err1;
      auto minerr = err2.minPos();
      rangeErr += minerr.get64() & 0xFFFF;
    }

    if(rangeErr < err)
    {
      err = rangeErr;
      sel = r;
      if(err == 0)
        break;
    }
  }

  auto recVal16 = rangeMul[sel];

  // find indices
  uint64_t idx = 0;

  for(int i = 0; i < 16; ++i)
  {
    auto err1 = sr[i] - recVal16;
    auto err2 = err1 * err1;
    idx |= (err2.minPos().get64() >> 16) << (15 - i) * 3;
  }

  std::array<uint16_t, sizeof(__m128i) / sizeof(uint16_t)> rm;
  mul.storeu(reinterpret_cast<__m128i*>(rm.data()));
  const uint16_t sm = srcMid.get64();

  const uint64_t d = (uint64_t(sm) << 56u) | (uint64_t(rm[getMulSel(sel)]) << 52u) | (uint64_t(sel) << 48u) | idx;

  return _bswap64(d);
}
} // namespace

void compressEtc2Bgra(const uint32_t* srcBgra, uint64_t* dst, uint32_t blocks, size_t width, bool useHeuristics)
{
  int w = 0;
  while(blocks--)
  {
    auto c0 = IVec8{reinterpret_cast<const __m128i*>(srcBgra + width * 0)};
    auto c1 = IVec8{reinterpret_cast<const __m128i*>(srcBgra + width * 1)};
    auto c2 = IVec8{reinterpret_cast<const __m128i*>(srcBgra + width * 2)};
    auto c3 = IVec8{reinterpret_cast<const __m128i*>(srcBgra + width * 3)};

    transpose(c0, c1, c2, c3);

    const auto immRgba = BgraBlockImm{{c0, c1, c2, c3}};

    BgraVecBlock bgra;
    c0.storeu(reinterpret_cast<__m128i*>(bgra.data()) + 0);
    c1.storeu(reinterpret_cast<__m128i*>(bgra.data()) + 1);
    c2.storeu(reinterpret_cast<__m128i*>(bgra.data()) + 2);
    c3.storeu(reinterpret_cast<__m128i*>(bgra.data()) + 3);

    static const auto mask0 = IVec{0x0f'0b'07'03, -1, -1, -1};
    static const auto mask1 = mask0.shuffled<3, 3, 0, 3>();
    static const auto mask2 = mask0.shuffled<3, 0, 3, 3>();
    static const auto mask3 = mask0.shuffled<0, 3, 3, 3>();

    const auto alphas = c0.shuffled(mask0) | c1.shuffled(mask1) | c2.shuffled(mask2) | c3.shuffled(mask3);

    srcBgra += 4;
    if(++w == width / 4)
    {
      srcBgra += width * 3;
      w = 0;
    }
    *dst++ = processAlpha_ETC2(alphas);
    *dst++ = processBGR_ETC2(bgra, immRgba, useHeuristics);
  }
}
