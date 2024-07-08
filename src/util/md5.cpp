/* MD5
 converted to C++ class by Frank Thilo (thilo@unix-ag.org)
 for bzflag (http://www.bzflag.org)

   based on:

   md5.h and md5.c
   reference implemantion of RFC 1321

   Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.

*/

#include "md5.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <gsl/gsl-lite.hpp>
#include <string>

constexpr size_t Blocksize = 64;

// Constants for MD5Transform routine.
enum : uint8_t
{
  S11 = 7,
  S12 = 12,
  S13 = 17,
  S14 = 22,
  S21 = 5,
  S22 = 9,
  S23 = 14,
  S24 = 20,
  S31 = 4,
  S32 = 11,
  S33 = 16,
  S34 = 23,
  S41 = 6,
  S42 = 10,
  S43 = 15,
  S44 = 21,
};

///////////////////////////////////////////////

// F, G, H and I are basic MD5 functions.
inline uint32_t F(const uint32_t x, const uint32_t y, const uint32_t z)
{
  return (x & y) | (~x & z);
}

inline uint32_t G(const uint32_t x, const uint32_t y, const uint32_t z)
{
  return (x & z) | (y & ~z);
}

inline uint32_t H(const uint32_t x, const uint32_t y, const uint32_t z)
{
  return x ^ y ^ z;
}

inline uint32_t I(const uint32_t x, const uint32_t y, const uint32_t z)
{
  return y ^ (x | ~z);
}

// rotate_left rotates x left n bits.
inline uint32_t rotate_left(const uint32_t x, const uint8_t n)
{
  return (x << n) | (x >> (32u - n));
}

// FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
// Rotation is separate from addition to prevent recomputation.
inline void FF(uint32_t& a,
               const uint32_t b,
               const uint32_t c,
               const uint32_t d,
               const uint32_t x,
               const uint8_t s,
               const uint32_t ac)
{
  a = rotate_left(a + F(b, c, d) + x + ac, s) + b;
}

inline void GG(uint32_t& a,
               const uint32_t b,
               const uint32_t c,
               const uint32_t d,
               const uint32_t x,
               const uint8_t s,
               const uint32_t ac)
{
  a = rotate_left(a + G(b, c, d) + x + ac, s) + b;
}

inline void HH(uint32_t& a,
               const uint32_t b,
               const uint32_t c,
               const uint32_t d,
               const uint32_t x,
               const uint8_t s,
               const uint32_t ac)
{
  a = rotate_left(a + H(b, c, d) + x + ac, s) + b;
}

inline void II(uint32_t& a,
               const uint32_t b,
               const uint32_t c,
               const uint32_t d,
               const uint32_t x,
               const uint8_t s,
               const uint32_t ac)
{
  a = rotate_left(a + I(b, c, d) + x + ac, s) + b;
}

//////////////////////////////

// decodes input (unsigned char) into output (uint32_t). Assumes len is a multiple of 4.
void decode(uint32_t* output, const uint8_t* input, const size_t len)
{
  for(size_t i = 0, j = 0; j < len; i++, j += 4)
  {
    output[i] = static_cast<uint32_t>(input[j]) | (static_cast<uint32_t>(input[j + 1]) << 8u)
                | (static_cast<uint32_t>(input[j + 2]) << 16u) | (static_cast<uint32_t>(input[j + 3]) << 24u);
  }
}

//////////////////////////////

// encodes input (uint32_t) into output (unsigned char). Assumes len is
// a multiple of 4.
void encode(uint8_t* output, const uint32_t* input, const size_t len)
{
  for(size_t i = 0, j = 0; j < len; i++, j += 4)
  {
    output[j] = static_cast<uint8_t>(input[i] & 0xffu);
    output[j + 1] = static_cast<uint8_t>((input[i] >> 8u) & 0xffu);
    output[j + 2] = static_cast<uint8_t>((input[i] >> 16u) & 0xffu);
    output[j + 3] = static_cast<uint8_t>((input[i] >> 24u) & 0xffu);
  }
}

//////////////////////////////

struct State
{
  bool finalized = false;
  std::array<uint8_t, Blocksize> buffer{}; // bytes that didn't fit in last 64 byte chunk
  std::array<uint32_t, 2> count{0, 0};     // 64bit counter for number of bits (lo, hi)
  std::array<uint32_t, 4> state{0x67452301u, 0xefcdab89u, 0x98badcfeu, 0x10325476u}; // digest so far
  std::array<uint8_t, 16> digest{};                                                  // the result

  // MD5 finalization. Ends an MD5 message-digest operation, writing the
  // the message digest and zeroizing the context.
  void finalize()
  {
    static const std::array<uint8_t, 64> padding{0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    if(!finalized)
    {
      // Save number of bits
      std::array<uint8_t, 8> bits{};
      encode(bits.data(), count.data(), 8);

      // pad out to 56 mod 64.
      const size_t index = count[0] / 8 % 64;
      const size_t padLen = (index < 56) ? (56 - index) : (120 - index);
      update(padding.data(), padLen);

      // Append length (before padding)
      update(bits.data(), 8);

      // Store state in digest
      encode(digest.data(), state.data(), 16);

      // Zeroize sensitive information.
      buffer.fill(0);
      count.fill(0);

      finalized = true;
    }
  }

  // apply MD5 algo on a block
  void transform(const uint8_t block[Blocksize])
  {
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    std::array<uint32_t, 16> x{};
    decode(x.data(), block, Blocksize);

    /* Round 1 */
    FF(a, b, c, d, x[0], S11, 0xd76aa478);  /* 1 */
    FF(d, a, b, c, x[1], S12, 0xe8c7b756);  /* 2 */
    FF(c, d, a, b, x[2], S13, 0x242070db);  /* 3 */
    FF(b, c, d, a, x[3], S14, 0xc1bdceee);  /* 4 */
    FF(a, b, c, d, x[4], S11, 0xf57c0faf);  /* 5 */
    FF(d, a, b, c, x[5], S12, 0x4787c62a);  /* 6 */
    FF(c, d, a, b, x[6], S13, 0xa8304613);  /* 7 */
    FF(b, c, d, a, x[7], S14, 0xfd469501);  /* 8 */
    FF(a, b, c, d, x[8], S11, 0x698098d8);  /* 9 */
    FF(d, a, b, c, x[9], S12, 0x8b44f7af);  /* 10 */
    FF(c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
    FF(b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
    FF(a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
    FF(d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
    FF(c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
    FF(b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

    /* Round 2 */
    GG(a, b, c, d, x[1], S21, 0xf61e2562);  /* 17 */
    GG(d, a, b, c, x[6], S22, 0xc040b340);  /* 18 */
    GG(c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
    GG(b, c, d, a, x[0], S24, 0xe9b6c7aa);  /* 20 */
    GG(a, b, c, d, x[5], S21, 0xd62f105d);  /* 21 */
    GG(d, a, b, c, x[10], S22, 0x2441453);  /* 22 */
    GG(c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
    GG(b, c, d, a, x[4], S24, 0xe7d3fbc8);  /* 24 */
    GG(a, b, c, d, x[9], S21, 0x21e1cde6);  /* 25 */
    GG(d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
    GG(c, d, a, b, x[3], S23, 0xf4d50d87);  /* 27 */
    GG(b, c, d, a, x[8], S24, 0x455a14ed);  /* 28 */
    GG(a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
    GG(d, a, b, c, x[2], S22, 0xfcefa3f8);  /* 30 */
    GG(c, d, a, b, x[7], S23, 0x676f02d9);  /* 31 */
    GG(b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

    /* Round 3 */
    HH(a, b, c, d, x[5], S31, 0xfffa3942);  /* 33 */
    HH(d, a, b, c, x[8], S32, 0x8771f681);  /* 34 */
    HH(c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
    HH(b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
    HH(a, b, c, d, x[1], S31, 0xa4beea44);  /* 37 */
    HH(d, a, b, c, x[4], S32, 0x4bdecfa9);  /* 38 */
    HH(c, d, a, b, x[7], S33, 0xf6bb4b60);  /* 39 */
    HH(b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
    HH(a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
    HH(d, a, b, c, x[0], S32, 0xeaa127fa);  /* 42 */
    HH(c, d, a, b, x[3], S33, 0xd4ef3085);  /* 43 */
    HH(b, c, d, a, x[6], S34, 0x4881d05);   /* 44 */
    HH(a, b, c, d, x[9], S31, 0xd9d4d039);  /* 45 */
    HH(d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
    HH(c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
    HH(b, c, d, a, x[2], S34, 0xc4ac5665);  /* 48 */

    /* Round 4 */
    II(a, b, c, d, x[0], S41, 0xf4292244);  /* 49 */
    II(d, a, b, c, x[7], S42, 0x432aff97);  /* 50 */
    II(c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
    II(b, c, d, a, x[5], S44, 0xfc93a039);  /* 52 */
    II(a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
    II(d, a, b, c, x[3], S42, 0x8f0ccc92);  /* 54 */
    II(c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
    II(b, c, d, a, x[1], S44, 0x85845dd1);  /* 56 */
    II(a, b, c, d, x[8], S41, 0x6fa87e4f);  /* 57 */
    II(d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
    II(c, d, a, b, x[6], S43, 0xa3014314);  /* 59 */
    II(b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
    II(a, b, c, d, x[4], S41, 0xf7537e82);  /* 61 */
    II(d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
    II(c, d, a, b, x[2], S43, 0x2ad7d2bb);  /* 63 */
    II(b, c, d, a, x[9], S44, 0xeb86d391);  /* 64 */

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;

    // Zeroize sensitive information.
    x.fill(0);
  }

  // MD5 block update operation. Continues an MD5 message-digest
  // operation, processing another message block
  void update(const uint8_t* input, const size_t length)
  {
    // compute number of bytes mod 64
    size_t index = count[0] / 8 % Blocksize;

    // Update number of bits
    const auto nBits = gsl::narrow_cast<uint32_t>(length << 3u);
    count[0] += nBits;
    if(count[0] < nBits)
    {
      ++count[1];
    }
    count[1] += gsl::narrow_cast<uint32_t>(length >> 29u);

    // number of bytes we need to fill in buffer
    const size_t firstpart = 64 - index;

    size_t i;

    // transform as many times as possible.
    if(length >= firstpart)
    {
      // fill buffer first, transform
      std::copy(&input[0], &input[firstpart], &buffer[index]);
      transform(buffer.data());

      // transform chunks of Blocksize (64 bytes)
      for(i = firstpart; i + Blocksize <= length; i += Blocksize)
      {
        transform(&input[i]);
      }

      index = 0;
    }
    else
    {
      i = 0;
    }

    // buffer remaining input
    std::copy(&input[i], &input[length], &buffer[index]);
  }

  // return hex representation of digest as string
  [[nodiscard]] std::string hexdigest() const
  {
    if(!finalized)
    {
      return {};
    }

    std::array<char, 33> buf{};
    buf.fill('\0');
    for(size_t i = 0; i < 16; i++)
    {
#ifdef WIN32
      gsl_Assert(sprintf_s(buf.data() + i * 2, 3, "%02X", digest[i]) == 2);
#else
      sprintf(buf.data() + i * 2, "%02X", digest[i]);
#endif
    }
    buf[32] = 0;

    return buf.data();
  }
};

std::string util::md5(const uint8_t* data, const size_t length)
{
  State state;
  state.update(data, length);
  state.finalize();
  return state.hexdigest();
}
