#pragma once

#include <cstddef>
#include <cstdint>
#include <gsl/gsl-lite.hpp>
#include <ios>

namespace image
{
// mode 1: 12 sync bytes + 4 header bytes, then 2048 bytes of user data, then 288 bytes of validation data, total 2352 bytes
// mode 2: 12 sync bytes + 4 header bytes, then 2336 bytes of user data, total 2352 bytes
// mode 2 XA form 1: basically the same as mode 1, with an additional 8-byte sub-header
// mode 2 XA form 2: somewhat like mode 2, with an additional 8-byte sub-header, but with 2324 bytes of user data
// mode 2 can never occur in CUE files

constexpr std::streamsize Mode1HeaderSize = 16;
constexpr std::streamsize Mode1UserDataSize = 2048;

constexpr std::streamsize Mode2XaHeaderSize = 24;
constexpr std::streamsize Mode2Form1UserDataSize = 2048;
constexpr std::streamsize Mode2Form2UserDataSize = 2324;

[[nodiscard]] inline std::streamsize getSectorHeaderSize(size_t sectorSize, bool mode2xa)
{
  if(sectorSize == Mode1UserDataSize)
    return 0;
  if(mode2xa)
    return Mode2XaHeaderSize;
  else
    return Mode1HeaderSize;
}

[[nodiscard]] inline std::streamsize getSectorUserDataSize(const gsl::span<uint8_t>& sector, bool mode2xa)
{
  if(!mode2xa)
    return Mode1UserDataSize;

  gsl_Assert(sector[18] != sector[22]);
  gsl_Assert(sector[18] == 0x08 || sector[18] == 0x28);
  return sector[18] == 0x08 ? Mode2Form1UserDataSize : Mode2Form2UserDataSize;
}
} // namespace image
