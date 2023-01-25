#pragma once

#include <cstdint>
#include <emmintrin.h>
#include <immintrin.h>
#include <smmintrin.h>
#include <tmmintrin.h>

struct IVec8;
struct IVec16;

struct IVec final
{
  explicit IVec()
      : data{_mm_setzero_si128()}
  {
  }

  template<typename T>
  explicit IVec(const T* unaligned)
      // NOLINTNEXTLINE cppcoreguidelines-pro-type-reinterpret-cast
      : data{_mm_loadu_si128(reinterpret_cast<const __m128i*>(unaligned))}
  {
    static_assert(sizeof(T) == sizeof(__m128i));
  }

  explicit IVec(__m128i x)
      : data{x}
  {
  }

  explicit IVec(int32_t x)
      : data{_mm_set1_epi32(x)}
  {
  }

  explicit IVec(int32_t x, int32_t y, int32_t z, int32_t w)
      : data{_mm_setr_epi32(x, y, z, w)}
  {
  }

  IVec(const IVec&) = default;
  IVec& operator=(const IVec&) = default;

  [[nodiscard]] IVec operator|(const IVec& rhs) const
  {
    return IVec{_mm_or_si128(data, rhs.data)};
  }

  [[nodiscard]] IVec operator&(const IVec& rhs) const
  {
    return IVec{_mm_and_si128(data, rhs.data)};
  }

  [[nodiscard]] IVec operator+(const IVec& rhs) const
  {
    return IVec{_mm_add_epi32(data, rhs.data)};
  }

  IVec& operator+=(const IVec& rhs)
  {
    *this = *this + rhs;
    return *this;
  }

  [[nodiscard]] IVec operator-(const IVec& rhs) const
  {
    return IVec{_mm_sub_epi32(data, rhs.data)};
  }

  [[nodiscard]] IVec operator*(const IVec& rhs) const
  {
    return IVec{_mm_mul_epi32(data, rhs.data)};
  }

  [[nodiscard]] IVec operator>>(int n) const
  {
    return IVec{_mm_srli_epi32(data, n)};
  }

  [[nodiscard]] IVec operator<(const IVec& rhs) const
  {
    return IVec{_mm_cmplt_epi32(data, rhs.data)};
  }

  void storeu(__m128i* unaligned) const
  {
    _mm_storeu_si128(unaligned, data);
  }

  template<uint8_t r3, uint8_t r2, uint8_t r1, uint8_t r0>
  [[nodiscard]] IVec shuffled() const
  {
    static_assert(r3 < 4);
    static_assert(r2 < 4);
    static_assert(r1 < 4);
    static_assert(r0 < 4);
    return IVec{_mm_shuffle_epi32(data, _MM_SHUFFLE(r3, r2, r1, r0))};
  }

  [[nodiscard]] auto abs() const
  {
    return IVec{_mm_abs_epi32(data)};
  }

  [[nodiscard]] auto min(const IVec& rhs) const
  {
    return IVec{_mm_min_epi32(data, rhs.data)};
  }

  [[nodiscard]] auto max(const IVec& rhs) const
  {
    return IVec{_mm_max_epi32(data, rhs.data)};
  }

  [[nodiscard]] auto dot(const IVec& rhs) const
  {
    const auto tmp = *this * rhs;
    const auto tmp2 = _mm_hadd_epi32(tmp.data, tmp.data);
    const auto tmp3 = _mm_hadd_epi32(tmp2, tmp2);
    return _mm_cvtsi128_si32(tmp3);
  }

  [[nodiscard]] inline IVec16 toIVec16U(const IVec& high) const;

  __m128i data;
};

struct IVec16 final
{
  explicit IVec16(__m128i x) noexcept
      : data{x}
  {
  }

  explicit IVec16(int16_t x = 0) noexcept
      : data{_mm_set1_epi16(x)}
  {
  }

  explicit IVec16(
    int16_t x0, int16_t x1, int16_t x2, int16_t x3, int16_t x4, int16_t x5, int16_t x6, int16_t x7) noexcept
      : data{_mm_setr_epi16(x0, x1, x2, x3, x4, x5, x6, x7)}
  {
  }

  IVec16(std::nullptr_t) = delete;

  explicit IVec16(const __m128i* unaligned) noexcept
      : data{_mm_loadu_si128(unaligned)}
  {
  }

  template<typename T>
  void storeu(T* unaligned) const noexcept
  {
    static_assert(sizeof(T) == sizeof(__m128i));
    // NOLINTNEXTLINE cppcoreguidelines-pro-type-reinterpret-cast
    _mm_storeu_si128(reinterpret_cast<__m128i*>(unaligned), data);
  }

  [[nodiscard]] IVec16 operator+(const IVec16& rhs) const
  {
    return IVec16{_mm_add_epi16(data, rhs.data)};
  }

  [[nodiscard]] IVec16 operator-(const IVec16& rhs) const
  {
    return IVec16{_mm_sub_epi16(data, rhs.data)};
  }

  [[nodiscard]] IVec16 operator*(const IVec16& rhs) const
  {
    return IVec16{_mm_mullo_epi16(data, rhs.data)};
  }

  [[nodiscard]] IVec16 mulHi(const IVec16& rhs) const
  {
    return IVec16{_mm_mulhi_epi16(data, rhs.data)};
  }

  [[nodiscard]] IVec16 operator|(const IVec16& rhs) const
  {
    return IVec16{_mm_or_si128(data, rhs.data)};
  }

  [[nodiscard]] IVec16 operator&(const IVec16& rhs) const
  {
    return IVec16{_mm_and_si128(data, rhs.data)};
  }

  [[nodiscard]] IVec16 operator>>(int n) const
  {
    return IVec16{_mm_srli_epi16(data, n)};
  }

  [[nodiscard]] IVec16 sra(int n) const
  {
    return IVec16{_mm_srai_epi16(data, n)};
  }

  [[nodiscard]] IVec16 operator<<(int n) const
  {
    return IVec16{_mm_slli_epi16(data, n)};
  }

  [[nodiscard]] IVec16 operator<(const IVec16& rhs) const
  {
    return IVec16{_mm_cmplt_epi16(data, rhs.data)};
  }

  [[nodiscard]] IVec lowToIVec32(const IVec16& rhs) const
  {
    return IVec{_mm_unpacklo_epi16(data, rhs.data)};
  }

  [[nodiscard]] inline IVec lowToIVec32() const;

  [[nodiscard]] IVec highToIVec32(const IVec16& rhs) const
  {
    return IVec{_mm_unpackhi_epi16(data, rhs.data)};
  }

  [[nodiscard]] inline IVec highToIVec32() const;

  [[nodiscard]] auto min(const IVec16& rhs) const
  {
    return IVec16{_mm_min_epi16(data, rhs.data)};
  }

  [[nodiscard]] auto max(const IVec16& rhs) const
  {
    return IVec16{_mm_max_epi16(data, rhs.data)};
  }

  template<int mask>
  [[nodiscard]] auto blended(const IVec16& value) const
  {
    return IVec16{_mm_blend_epi16(data, value.data, mask)};
  }

  template<uint8_t a, uint8_t b, uint8_t c, uint8_t d>
  auto& shuffle32()
  {
    static_assert(a < 4);
    static_assert(b < 4);
    static_assert(c < 4);
    static_assert(d < 4);
    data = _mm_shuffle_epi32(data, _MM_SHUFFLE(a, b, c, d));
    return *this;
  }

  template<uint8_t a, uint8_t b, uint8_t c, uint8_t d>
  [[nodiscard]] IVec16 shuffled32() const
  {
    auto tmp{*this};
    tmp.shuffle32<a, b, c, d>();
    return tmp;
  }

  [[nodiscard]] auto minPos() const
  {
    return IVec16{_mm_minpos_epu16(data)};
  }

  [[nodiscard]] inline IVec8 toIVec8U(const IVec16& high) const;

  [[nodiscard]] auto get64() const
  {
    return _mm_cvtsi128_si64(data);
  }

  [[nodiscard]] auto abs() const
  {
    return IVec16{_mm_abs_epi16(data)};
  }

  __m128i data;
};

struct IVec8 final
{
  explicit IVec8(__m128i x)
      : data{x}
  {
  }

  explicit IVec8(const __m128i* unaligned)
      : data{_mm_loadu_si128(unaligned)}
  {
  }

  explicit IVec8(int8_t x)
      : data{_mm_set1_epi8(x)}
  {
  }

  explicit IVec8(uint8_t x)
      : data{_mm_set1_epi8(x)}
  {
  }

  template<typename T>
  void storeu(T* unaligned) const
  {
    static_assert(sizeof(T) == sizeof(__m128i));
    // NOLINTNEXTLINE cppcoreguidelines-pro-type-reinterpret-cast
    _mm_storeu_si128(reinterpret_cast<__m128i*>(unaligned), data);
  }

  [[nodiscard]] auto operator==(const IVec8& rhs) const
  {
    return IVec8{_mm_cmpeq_epi8(data, rhs.data)};
  }

  [[nodiscard]] auto operator&(int rhs) const
  {
    return IVec8{_mm_and_si128(data, _mm_set1_epi32(rhs))};
  }

  [[nodiscard]] auto operator|(const IVec8& rhs) const
  {
    return IVec8{_mm_or_si128(data, rhs.data)};
  }

  [[nodiscard]] auto operator&(const IVec8& rhs) const
  {
    return IVec8{_mm_and_si128(data, rhs.data)};
  }

  [[nodiscard]] IVec16 lowToIVec16() const
  {
    return IVec16{_mm_unpacklo_epi8(data, _mm_setzero_si128())};
  }

  [[nodiscard]] IVec16 highToIVec16() const
  {
    return IVec16{_mm_unpackhi_epi8(data, _mm_setzero_si128())};
  }

  template<uint8_t a, uint8_t b, uint8_t c, uint8_t d>
  auto& shuffle32()
  {
    static_assert(a < 4);
    static_assert(b < 4);
    static_assert(c < 4);
    static_assert(d < 4);
    data = _mm_shuffle_epi32(data, _MM_SHUFFLE(a, b, c, d));
    return *this;
  }

  template<uint8_t a, uint8_t b, uint8_t c, uint8_t d>
  [[nodiscard]] IVec8 shuffled32() const
  {
    auto tmp{*this};
    tmp.shuffle32<a, b, c, d>();
    return tmp;
  }

  [[nodiscard]] auto umin(const IVec8& rhs) const
  {
    return IVec8{_mm_min_epu8(data, rhs.data)};
  }

  [[nodiscard]] auto umax(const IVec8& rhs) const
  {
    return IVec8{_mm_max_epu8(data, rhs.data)};
  }

  template<int n>
  [[nodiscard]] auto alignR(const IVec8& rhs) const
  {
    return IVec8{_mm_alignr_epi8(data, rhs.data, n)};
  }

  [[nodiscard]] auto testC(const IVec& rhs) const
  {
    return _mm_testc_si128(data, rhs.data);
  }

  [[nodiscard]] auto get32() const
  {
    return _mm_cvtsi128_si32(data);
  }

  [[nodiscard]] IVec8 shuffled(const IVec& rhs) const
  {
    return IVec8{_mm_shuffle_epi8(data, rhs.data)};
  }

  __m128i data;
};

IVec16 IVec::toIVec16U(const IVec& high) const
{
  return IVec16{_mm_packus_epi32(data, high.data)};
}

IVec8 IVec16::toIVec8U(const IVec16& high) const
{
  return IVec8{_mm_packus_epi16(data, high.data)};
}

IVec IVec16::lowToIVec32() const
{
  return lowToIVec32(IVec16{int16_t(0)});
}

IVec IVec16::highToIVec32() const
{
  return highToIVec32(IVec16{int16_t(0)});
}

inline void transpose(IVec8& a, IVec8& b, IVec8& c, IVec8& d)
{
  auto ta = _mm_castsi128_ps(a.data);
  auto tb = _mm_castsi128_ps(b.data);
  auto tc = _mm_castsi128_ps(c.data);
  auto td = _mm_castsi128_ps(d.data);
  _MM_TRANSPOSE4_PS(ta, tb, tc, td);
  a.data = _mm_castps_si128(ta);
  b.data = _mm_castps_si128(tb);
  c.data = _mm_castps_si128(tc);
  d.data = _mm_castps_si128(td);
}
