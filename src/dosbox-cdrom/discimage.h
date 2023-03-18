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

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <vector>

namespace image
{
class BinaryFile;
struct Track;

class DiscImage final
{
public:
  explicit DiscImage(const std::filesystem::path& cueFilepath);
  ~DiscImage();
  [[nodiscard]] std::vector<uint8_t> read(size_t sector, size_t size);
  [[nodiscard]] std::vector<uint8_t> readSector(size_t sector);

private:
  [[nodiscard]] const Track* getTrackForSector(size_t sector);

  std::vector<Track> m_tracks;
};
} // namespace image

#endif /* __CDROM_INTERFACE__ */
