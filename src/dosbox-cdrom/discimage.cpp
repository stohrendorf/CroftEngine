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
#include "cueparser.h"
#include "formatutils.h"
#include "physicaltrackbuilder.h"

#include <array>
#include <boost/log/trivial.hpp>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <vector>

namespace cdrom
{
DiscImage::~DiscImage() = default;

bool DiscImage::read(std::vector<uint8_t>& buffer, size_t sector, size_t size)
{
  buffer.clear();
  buffer.reserve(size);

  while(buffer.size() < size)
  {
    auto tmp = readSector(sector);
    if(tmp.empty())
    {
      BOOST_LOG_TRIVIAL(warning) << "failed to read sector " << sector;
      return false;
    }
    if(tmp.size() > size - buffer.size())
    {
      tmp.resize(size - buffer.size());
    }

    buffer.insert(buffer.end(), tmp.cbegin(), tmp.cend());
    ++sector;
  }

  return true;
}

const Track* DiscImage::getTrackForSector(size_t sector)
{
  for(const auto& track : m_tracks)
  {
    if(track.startSector <= sector && sector < track.startSector + track.totalSectors)
      return &track;
  }
  return nullptr;
}

std::vector<uint8_t> DiscImage::readSector(size_t sector)
{
  const auto track = getTrackForSector(sector);
  if(track == nullptr)
  {
    BOOST_LOG_TRIVIAL(warning) << "no track found for sector " << sector;
    return {};
  }

  const auto sectorOffset = track->fileOffset + (sector - track->startSector) * track->sectorSize;
  std::vector<uint8_t> data;
  data.resize(Mode2XaHeaderSize);
  track->file->read(data, sectorOffset);
  data.resize(getSectorUserDataSize(data, track->mode2xa));
  track->file->read(data, sectorOffset + getSectorHeaderSize(track->sectorSize, track->mode2xa));
  return data;
}

DiscImage::DiscImage(const std::filesystem::path& cueFilepath)
    : m_tracks{PhysicalTrackBuilder{cueFilepath}.getTracks()}
{
}
} // namespace cdrom
