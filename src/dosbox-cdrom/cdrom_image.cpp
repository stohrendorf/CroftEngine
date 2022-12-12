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

#include "cdrom.h"

#include "binaryfile.h"
#include "cueparser.h"
#include "formatutils.h"

#include <array>
#include <boost/log/trivial.hpp>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <vector>

namespace cdrom
{
CdImage::~CdImage() = default;

bool CdImage::read(std::vector<uint8_t>& buffer, size_t sector, std::streamsize size)
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

    std::copy_n(tmp.cbegin(), std::min(size - buffer.size(), tmp.size()), std::back_inserter(buffer));
    ++sector;
  }

  return true;
}

const CdImage::Track* CdImage::getTrackForSector(size_t sector)
{
  for(const auto& track : m_tracks)
  {
    if(track.startSector <= sector && sector < track.startSector + track.totalSectors)
      return &track;
  }
  return nullptr;
}

std::vector<uint8_t> CdImage::readSector(size_t sector)
{
  const auto track = getTrackForSector(sector);
  if(track == nullptr)
    return {};

  const std::streamoff sectorOffset = track->fileOffset + (sector - track->startSector) * track->sectorSize;
  std::vector<uint8_t> data;
  data.resize(Mode2XaHeaderSize);
  track->file->read(data, sectorOffset);
  data.resize(getSectorUserDataSize(data, track->mode2xa));
  track->file->read(data, sectorOffset + getSectorHeaderSize(track->sectorSize, track->mode2xa));
  return data;
}

bool CdImage::loadIsoFile(const std::filesystem::path& filename)
{
  m_tracks.clear();

  // data track
  Track track{};
  track.file = std::make_shared<BinaryFile>(filename);

  // try to detect iso type
  if(containsPrimaryVolumeDescriptor(*track.file, Mode1UserDataSize, false))
  {
    // mode 1, only user data
    track.sectorSize = 2048;
    track.mode2xa = false;
  }
  else if(containsPrimaryVolumeDescriptor(*track.file, Mode1SectorSize, false))
  {
    // mode 1, full CD sectors
    track.sectorSize = Mode1SectorSize;
    track.mode2xa = false;
  }
  else if(containsPrimaryVolumeDescriptor(*track.file, Mode2XaHeaderlessSectorSize, true))
  {
    // mode 2 xa, only user data
    track.sectorSize = Mode2XaHeaderlessSectorSize;
    track.mode2xa = true;
  }
  else if(containsPrimaryVolumeDescriptor(*track.file, Mode2XaSectorSize, true))
  {
    // mode 2 xa, full CD sectors
    track.sectorSize = Mode2XaSectorSize;
    track.mode2xa = true;
  }
  else
  {
    BOOST_LOG_TRIVIAL(error) << "failed to detect iso type";
    return false;
  }

  track.totalSectors = track.file->size() / track.sectorSize;
  m_tracks.push_back(track);

  return true;
}

bool CdImage::loadCueSheet(const std::filesystem::path& cuefile)
{
  m_tracks.clear();
  size_t discSectorStart = 0;
  size_t totalPregap = 0;

  const auto tracks = cue::readCueSheet(cuefile);
  std::shared_ptr<BinaryFile> file;
  for(const auto& track : tracks)
  {
    if(file == nullptr || track.filepath != file->getFilepath())
    {
      gsl_Assert(!track.filepath.empty());
      file = std::make_shared<BinaryFile>(track.filepath);
    }
    if(!addTrack(track, discSectorStart, totalPregap, file))
      return false;
  }

  return true;
}

bool CdImage::addTrack(const cue::Track& curr,
                       size_t& discSectorStart,
                       size_t& totalPregap,
                       const std::shared_ptr<BinaryFile>& file)
{
  if(curr.index != m_tracks.size() + 1)
  {
    BOOST_LOG_TRIVIAL(error) << "invalid track number";
    return false;
  }

  Track track{curr.start, curr.sectorSize, curr.mode2xa, file};
  size_t fileOffsetSectors = 0;
  if(curr.pregapStart > 0)
  {
    if(curr.pregapStart > curr.start)
    {
      BOOST_LOG_TRIVIAL(error) << "invalid pregap start";
      return false;
    }
    fileOffsetSectors = curr.start - curr.pregapStart;
  }

  // first track (track number must be 1)
  if(m_tracks.empty())
  {
    track.fileOffset = fileOffsetSectors * curr.sectorSize;
    track.startSector += curr.pregap;
    totalPregap = curr.pregap;
    BOOST_LOG_TRIVIAL(debug) << "Initial track: startSector=" << track.startSector
                             << ", totalSectors=" << track.totalSectors << ", fileOffset=" << track.fileOffset
                             << ", sectorSize=" << track.sectorSize << ", mode2xa=" << track.mode2xa << ", file="
                             << (track.file == nullptr ? "<none>" : track.file->getFilepath().string().c_str());

    m_tracks.push_back(track);
    return true;
  }

  Track& prev = m_tracks.back();

  // current track consumes data from the same file as the previous
  if(prev.file == file)
  {
    track.startSector += discSectorStart;
    prev.totalSectors = track.startSector + totalPregap - prev.startSector - fileOffsetSectors;
    track.fileOffset += prev.fileOffset + prev.totalSectors * prev.sectorSize + fileOffsetSectors * curr.sectorSize;
    totalPregap += curr.pregap;
    track.startSector += totalPregap;
  }
  else
  {
    const auto remaining = prev.file->size() - prev.fileOffset;
    prev.totalSectors = remaining / prev.sectorSize;
    if(remaining % prev.sectorSize != 0)
      prev.totalSectors++; // padding

    track.startSector += prev.startSector + prev.totalSectors + curr.pregap;
    track.fileOffset = fileOffsetSectors * curr.sectorSize;
    discSectorStart += prev.startSector + prev.totalSectors;
    totalPregap = curr.pregap;
  }

  // error checks
  if(track.startSector < prev.startSector + prev.totalSectors)
  {
    BOOST_LOG_TRIVIAL(error) << "overlapping tracks";
    return false;
  }

  BOOST_LOG_TRIVIAL(debug) << "New track: startSector=" << track.startSector << ", totalSectors=" << track.totalSectors
                           << ", fileOffset=" << track.fileOffset << ", sectorSize=" << track.sectorSize
                           << ", mode2xa=" << track.mode2xa << ", file="
                           << (track.file == nullptr ? "<none>" : track.file->getFilepath().string().c_str());

  m_tracks.emplace_back(track);
  return true;
}

CdImage::CdImage(const std::filesystem::path& filename)
{
  if(loadCueSheet(filename))
    return;
  if(loadIsoFile(filename))
    return;

  BOOST_THROW_EXCEPTION(std::runtime_error("could not load image file"));
}
} // namespace cdrom
