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
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace cue
{
struct Track;
}

class BinaryFile;

namespace cdrom
{
class DiscImage final
{
private:
  struct Track
  {
    size_t startSector = 0;
    size_t sectorSize = 0;
    bool mode2xa = false;
    std::shared_ptr<BinaryFile> file{};
    size_t totalSectors = 0;
    size_t fileOffset = 0;
  };

public:
  explicit DiscImage(const std::filesystem::path& filename);
  ~DiscImage();
  [[nodiscard]] bool read(std::vector<uint8_t>& buffer, size_t sector, size_t size);
  [[nodiscard]] std::vector<uint8_t> readSector(size_t sector);

private:
  [[nodiscard]] const Track* getTrackForSector(size_t sector);

  [[nodiscard]] bool loadIsoFile(const std::filesystem::path& filename);
  [[nodiscard]] bool loadCueSheet(const std::filesystem::path& cuefile);
  [[nodiscard]] bool addTrack(const cue::Track& curr,
                              size_t& discSectorStart,
                              size_t& totalPregap,
                              const std::shared_ptr<BinaryFile>& file);

  std::vector<Track> m_tracks;
};
} // namespace cdrom

#endif /* __CDROM_INTERFACE__ */
