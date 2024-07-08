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

#include "discimage.h"

#include "binaryfile.h"
#include "formatutils.h"
#include "physicaltrackbuilder.h"

#include <algorithm>
#include <boost/log/trivial.hpp>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <vector>

namespace image
{
DiscImage::~DiscImage() = default;

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
std::vector<uint8_t> DiscImage::read(size_t sector, size_t size)
{
  std::vector<uint8_t> buffer;
  buffer.reserve(size);

  while(buffer.size() < size)
  {
    auto sectorData = readSector(sector);
    if(sectorData.empty())
    {
      BOOST_LOG_TRIVIAL(warning) << "failed to read sector " << sector;
      return {};
    }
    if(sectorData.size() > size - buffer.size())
    {
      sectorData.resize(size - buffer.size());
    }

    buffer.insert(buffer.end(), sectorData.cbegin(), sectorData.cend());
    ++sector;
  }

  return buffer;
}

const Track* DiscImage::getTrackForSector(size_t sector)
{
  auto it = std::find_if(m_tracks.begin(),
                         m_tracks.end(),
                         [sector](const auto& track)
                         {
                           return track.startSector <= sector && sector < track.startSector + track.totalSectors;
                         });
  return it == m_tracks.end() ? nullptr : &*it;
}

std::vector<uint8_t> DiscImage::readSector(size_t sector)
{
  const auto track = getTrackForSector(sector);
  if(track == nullptr)
  {
    BOOST_LOG_TRIVIAL(error) << "no track found for sector " << sector;
    return {};
  }

  const auto sectorOffset = track->fileOffset + (sector - track->startSector) * track->sectorSize;
  std::vector<uint8_t> data;
  data.resize(Mode2XaHeaderSize);
  if(!track->file->read(data, sectorOffset))
  {
    BOOST_LOG_TRIVIAL(error) << "failed to read sector header";
    return {};
  }
  data.resize(getSectorUserDataSize(data, track->mode2xa));
  if(!track->file->read(data, sectorOffset + getSectorHeaderSize(track->sectorSize, track->mode2xa)))
  {
    BOOST_LOG_TRIVIAL(error) << "failed to read sector data";
    return {};
  }
  return data;
}

DiscImage::DiscImage(const std::filesystem::path& cueFilepath)
    : m_tracks{PhysicalTrackBuilder{cueFilepath}.getTracks()}
{
}
} // namespace image
