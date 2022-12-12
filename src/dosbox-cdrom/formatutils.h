#pragma once

#include "binaryfile.h"

#include <boost/log/trivial.hpp>
#include <gsl/gsl-lite.hpp>
#include <ios>

// mode 1: 12 sync bytes + 4 header bytes, then 2048 bytes of user data, then 288 bytes of validation data, total 2352 bytes
// mode 2: 12 sync bytes + 4 header bytes, then 2336 bytes of user data, total 2352 bytes
// mode 2 XA form 1: basically the same as mode 1, with an additional 8-byte sub-header
// mode 2 XA form 2: somewhat like mode 2, with an additional 8-byte sub-header, but with 2324 bytes of user data
// mode 2 can never occur in CUE files

constexpr std::streamsize Mode1SectorSize = 2352;
constexpr std::streamsize Mode1HeaderSize = 16;
constexpr std::streamsize Mode1UserDataSize = 2048;

constexpr std::streamsize Mode2XaSectorSize = 2352;
constexpr std::streamsize Mode2XaHeaderlessSectorSize = 2336;
constexpr std::streamsize Mode2XaHeaderSize = 24;
constexpr std::streamsize Mode2Form1UserDataSize = 2048;
constexpr std::streamsize Mode2Form2UserDataSize = 2324;

[[nodiscard]] inline std::streamsize getSectorHeaderSize(std::streamsize sectorSize, bool mode2xa)
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

[[nodiscard]] inline bool containsPrimaryVolumeDescriptor(BinaryFile& file, std::streamsize sectorSize, bool mode2xa)
{
  std::array<uint8_t, 2048> pvd{};
  // sectors 0..15 are reserved sectors
  const std::streamoff seek = 16 * sectorSize + getSectorHeaderSize(sectorSize, mode2xa);
  if(!file.read(pvd, seek))
  {
    BOOST_LOG_TRIVIAL(error) << "failed to read " << pvd.size() << " bytes from " << seek;
    return false;
  }
  // pvd[0] = descriptor type, pvd[1..5] = standard identifier, pvd[6] = iso version (+8 for High Sierra)
  return ((pvd[0] == 1 && !strncmp((char*)(&pvd[1]), "CD001", 5) && pvd[6] == 1)
          || (pvd[8] == 1 && !strncmp((char*)(&pvd[9]), "CDROM", 5) && pvd[14] == 1));
}
