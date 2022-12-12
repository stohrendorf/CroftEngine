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

#include <array>
#include <boost/log/trivial.hpp>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <vector>

#define COOKED_SECTOR_SIZE 2048u
#define RAW_SECTOR_SIZE 2352u

namespace cdrom
{
namespace
{
bool canReadPVD(BinaryFile& file, int sectorSize, bool mode2)
{
  std::array<uint8_t, COOKED_SECTOR_SIZE> pvd{};
  int seek = 16 * sectorSize; // first vd is located at sector 16
  if(sectorSize == RAW_SECTOR_SIZE && !mode2)
    seek += 16;
  if(mode2)
    seek += 24;
  if(!file.read(gsl::span{pvd.data(), pvd.size() - 1}, seek))
  {
    BOOST_LOG_TRIVIAL(error) << "failed to read " << pvd.size() - 1 << " bytes from " << seek;
    return false;
  }
  // pvd[0] = descriptor type, pvd[1..5] = standard identifier, pvd[6] = iso version (+8 for High Sierra)
  return ((pvd[0] == 1 && !strncmp((char*)(&pvd[1]), "CD001", 5) && pvd[6] == 1)
          || (pvd[8] == 1 && !strncmp((char*)(&pvd[9]), "CDROM", 5) && pvd[14] == 1));
}
} // namespace

CdImage::~CdImage() = default;

bool CdImage::readSectors(std::vector<uint8_t>& buffer, size_t sector, size_t num)
{
  buffer.resize(num * COOKED_SECTOR_SIZE);

  for(size_t i = 0; i < num; i++)
  {
    if(!readSector(gsl::span{&buffer[i * COOKED_SECTOR_SIZE], COOKED_SECTOR_SIZE}, sector + i))
      return false;
  }

  return true;
}

bool CdImage::read(std::vector<uint8_t>& buffer, size_t sector, std::streamsize size)
{
  const size_t numSectors = (size + COOKED_SECTOR_SIZE - 1) / COOKED_SECTOR_SIZE;
  buffer.resize(numSectors * COOKED_SECTOR_SIZE);

  for(size_t i = 0; i < numSectors; i++)
  {
    if(!readSector(gsl::span{&buffer[i * COOKED_SECTOR_SIZE], COOKED_SECTOR_SIZE}, sector + i))
      return false;
  }

  buffer.resize(size);
  return true;
}

std::optional<size_t> CdImage::getTrack(size_t sector)
{
  for(auto it = m_tracks.cbegin(), end = std::prev(m_tracks.cend()); it != end; ++it)
  {
    if(it->startSector <= sector && sector < std::next(it)->startSector)
      return it->number;
  }
  return std::nullopt;
}

bool CdImage::readSector(const gsl::span<uint8_t>& buffer, size_t sector)
{
  const auto track = getTrack(sector);
  if(!track.has_value() || *track == 0)
    return false;

  const auto trackIdx = *track - 1;

  std::streamoff seek
    = m_tracks[trackIdx].fileOffset + (sector - m_tracks[trackIdx].startSector) * m_tracks[trackIdx].sectorSize;
  if(m_tracks[trackIdx].sectorSize == RAW_SECTOR_SIZE && !m_tracks[trackIdx].mode2)
    seek += 16;
  if(m_tracks[trackIdx].mode2)
    seek += 24;

  return m_tracks[trackIdx].file->read(buffer, seek);
}

bool CdImage::loadIsoFile(const std::filesystem::path& filename)
{
  m_tracks.clear();

  // data track
  Track track{};
  track.file = std::make_shared<BinaryFile>(filename);
  track.number = 1;

  // try to detect iso type
  if(canReadPVD(*track.file, COOKED_SECTOR_SIZE, false))
  {
    track.sectorSize = COOKED_SECTOR_SIZE;
    track.mode2 = false;
  }
  else if(canReadPVD(*track.file, RAW_SECTOR_SIZE, false))
  {
    track.sectorSize = RAW_SECTOR_SIZE;
    track.mode2 = false;
  }
  else if(canReadPVD(*track.file, 2336, true))
  {
    track.sectorSize = 2336;
    track.mode2 = true;
  }
  else if(canReadPVD(*track.file, RAW_SECTOR_SIZE, true))
  {
    track.sectorSize = RAW_SECTOR_SIZE;
    track.mode2 = true;
  }
  else
  {
    BOOST_LOG_TRIVIAL(error) << "failed to detect iso type";
    return false;
  }

  track.totalSectors = track.file->size() / track.sectorSize;
  m_tracks.push_back(track);

  // leadout track
  track.number = 2;
  track.startSector = track.totalSectors;
  track.totalSectors = 0;
  track.file = nullptr;
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

  // add leadout track
  cue::Track track;
  track.index = m_tracks.back().number + 1;
  if(!addTrack(track, discSectorStart, totalPregap, nullptr))
    return false;

  return true;
}

bool CdImage::addTrack(const cue::Track& curr,
                       size_t& discSectorStart,
                       size_t& totalPregap,
                       const std::shared_ptr<BinaryFile>& file)
{
  Track track{curr.index, curr.start, curr.sectorSize, curr.mode2, file};
  // frames between index 0(prestart) and 1(curr.start) must be skipped
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
    if(curr.index != 1)
    {
      BOOST_LOG_TRIVIAL(error) << "invalid initial track number";
      return false;
    }
    track.fileOffset = fileOffsetSectors * curr.sectorSize;
    track.startSector += curr.pregap;
    totalPregap = curr.pregap;
    BOOST_LOG_TRIVIAL(debug) << "Initial track: number=" << track.number << ", startSector=" << track.startSector
                             << ", totalSectors=" << track.totalSectors << ", fileOffset=" << track.fileOffset
                             << ", sectorSize=" << track.sectorSize << ", mode2=" << track.mode2 << ", file="
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
  if(curr.index <= 1)
  {
    BOOST_LOG_TRIVIAL(error) << "invalid track number";
    return false;
  }
  if(prev.number + 1 != curr.index)
  {
    BOOST_LOG_TRIVIAL(error) << "invalid track number sequence";
    return false;
  }
  if(track.startSector < prev.startSector + prev.totalSectors)
  {
    BOOST_LOG_TRIVIAL(error) << "overlapping tracks";
    return false;
  }

  BOOST_LOG_TRIVIAL(debug) << "New track: number=" << track.number << ", startSector=" << track.startSector
                           << ", totalSectors=" << track.totalSectors << ", fileOffset=" << track.fileOffset
                           << ", sectorSize=" << track.sectorSize << ", mode2=" << track.mode2 << ", file="
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
