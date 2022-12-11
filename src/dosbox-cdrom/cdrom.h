/*
 *  Copyright (C) 2002-2021  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __CDROM_INTERFACE__
#define __CDROM_INTERFACE__

#include <cstring>
#include <filesystem>
#include <fstream>
#include <gsl/gsl-lite.hpp>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace cdrom
{
class BinaryFile final
{
public:
  explicit BinaryFile(const std::filesystem::path& filename);
  BinaryFile() = delete;
  bool read(uint8_t* buffer, std::streampos seek, std::streamsize count);
  std::streamsize size();

private:
  std::ifstream m_file;
};

class CdImage
{
private:
  struct Track
  {
    size_t number = 0;
    size_t start = 0;
    size_t length = 0;
    size_t skip = 0;
    size_t sectorSize = 0;
    bool mode2 = false;
    std::shared_ptr<BinaryFile> file{};
  };

public:
  explicit CdImage(const std::filesystem::path& filename);
  ~CdImage() = default;
  bool readSectors(std::vector<uint8_t>& buffer, size_t sector, size_t num);
  bool read(std::vector<uint8_t>& buffer, size_t sector, std::streamsize size);
  bool readSector(const gsl::span<uint8_t>& buffer, size_t sector);

private:
  int getTrack(size_t sector);

  bool loadIsoFile(const std::filesystem::path& filename);
  bool loadCueSheet(const std::filesystem::path& cuefile);
  bool addTrack(Track& curr, size_t& shift, size_t prestart, size_t& totalPregap, size_t currPregap);

  std::vector<Track> m_tracks;
};
} // namespace cdrom

#endif /* __CDROM_INTERFACE__ */
