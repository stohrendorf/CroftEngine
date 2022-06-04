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

#include <array>
#include <boost/log/trivial.hpp>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <vector>

#define MAX_FILENAME_LENGTH 256u
#define COOKED_SECTOR_SIZE 2048u
#define RAW_SECTOR_SIZE 2352u

#define CD_FPS 75u
#define MSF_TO_FRAMES(M, S, F) ((M)*60u * CD_FPS + (S)*CD_FPS + (F))

namespace cdrom
{
namespace
{
std::string getCueKeyword(std::istream& in)
{
  std::string keyword;
  in >> keyword;
  for(char& i : keyword)
    i = gsl::narrow<char>(std::toupper(i));
  return keyword;
}

bool getCueFrame(size_t& frames, std::istream& in)
{
  std::string msf;
  in >> msf;
  int min, sec, fr;
  bool success = std::sscanf(msf.c_str(), "%d:%d:%d", &min, &sec, &fr) == 3;
  if(!success)
  {
    BOOST_LOG_TRIVIAL(error) << "could not parse cue frame " << msf;
  }
  frames = gsl::narrow<size_t>(MSF_TO_FRAMES(min, sec, fr));

  return success;
}

std::string getCueString(std::istream& in)
{
  const auto pos = in.tellg();
  std::string str;
  in >> str;
  if(str[0] != '\"')
    return str;

  if(str[str.size() - 1] == '\"')
  {
    str.assign(str, 1, str.size() - 2);
    return str;
  }

  in.seekg(pos, std::ios::beg);
  std::array<char, MAX_FILENAME_LENGTH> buffer{};
  in.getline(buffer.data(), buffer.size() - 1, '\"'); // skip
  in.getline(buffer.data(), buffer.size() - 1, '\"');
  return buffer.data();
}

bool canReadPVD(BinaryFile& file, int sectorSize, bool mode2)
{
  std::array<uint8_t, COOKED_SECTOR_SIZE> pvd{};
  int seek = 16 * sectorSize; // first vd is located at sector 16
  if(sectorSize == RAW_SECTOR_SIZE && !mode2)
    seek += 16;
  if(mode2)
    seek += 24;
  if(!file.read(pvd.data(), seek, pvd.size() - 1))
    BOOST_LOG_TRIVIAL(error) << "failed to read " << pvd.size() - 1 << " bytes from " << seek;
  // pvd[0] = descriptor type, pvd[1..5] = standard identifier, pvd[6] = iso version (+8 for High Sierra)
  return ((pvd[0] == 1 && !strncmp((char*)(&pvd[1]), "CD001", 5) && pvd[6] == 1)
          || (pvd[8] == 1 && !strncmp((char*)(&pvd[9]), "CDROM", 5) && pvd[14] == 1));
}

bool getRealFileName(std::string& filename, const std::filesystem::path& pathname)
{
  if(!std::filesystem::is_regular_file(pathname / filename))
    return false;

  filename = (pathname / filename).string();
  return true;
}
} // namespace

BinaryFile::BinaryFile(const std::filesystem::path& filename)
    : m_file{filename, std::ios::in | std::ios::binary}
{
  BOOST_LOG_TRIVIAL(debug) << "open " << filename;
  m_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  if(m_file.fail() || !m_file.is_open())
    BOOST_THROW_EXCEPTION(std::runtime_error("failed to open binary file"));
}

bool BinaryFile::read(uint8_t* buffer, std::streampos seek, std::streamsize count)
{
  std::fill_n(buffer, count, uint8_t{0});
  m_file.seekg(seek, std::ios::beg);
  m_file.read((char*)buffer, count);
  return !m_file.fail();
}

std::streamsize BinaryFile::size()
{
  m_file.seekg(0, std::ios::end);
  return m_file.tellg();
}

bool CdImage::readSectors(std::vector<uint8_t>& buffer, size_t sector, size_t num)
{
  buffer.resize(num * COOKED_SECTOR_SIZE);

  bool success = true; //Gobliiins reads 0 sectors
  for(size_t i = 0; i < num; i++)
  {
    success = readSector(gsl::span{&buffer[i * COOKED_SECTOR_SIZE], COOKED_SECTOR_SIZE}, sector + i);
    if(!success)
      break;
  }

  return success;
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

int CdImage::getTrack(size_t sector)
{
  auto i = m_tracks.begin();
  auto end = m_tracks.end() - 1;

  while(i != end)
  {
    Track& curr = *i;
    Track& next = *(i + 1);
    if(curr.start <= sector && sector < next.start)
      return curr.number;
    i++;
  }
  return -1;
}

bool CdImage::readSector(const gsl::span<uint8_t>& buffer, size_t sector)
{
  int track = getTrack(sector) - 1;
  if(track < 0)
    return false;

  size_t seek = m_tracks[track].skip + (sector - m_tracks[track].start) * m_tracks[track].sectorSize;
  if(m_tracks[track].sectorSize == RAW_SECTOR_SIZE && !m_tracks[track].mode2)
    seek += 16;
  if(m_tracks[track].mode2)
    seek += 24;

  return m_tracks[track].file->read(buffer.data(), seek, buffer.size());
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

  track.length = track.file->size() / track.sectorSize;
  m_tracks.push_back(track);

  // leadout track
  track.number = 2;
  track.start = track.length;
  track.length = 0;
  track.file = nullptr;
  m_tracks.push_back(track);

  return true;
}

bool CdImage::loadCueSheet(const std::filesystem::path& cuefile)
{
  Track track{};
  m_tracks.clear();
  size_t shift = 0;
  size_t currPregap = 0;
  size_t totalPregap = 0;
  size_t prestart = 0;
  bool success;
  bool canAddTrack = false;
  std::ifstream in;
  in.open(cuefile, std::ios::in);
  if(in.fail())
  {
    BOOST_LOG_TRIVIAL(error) << "could not open " << cuefile;
    return false;
  }

  while(!in.eof())
  {
    // get next line
    std::array<char, 512> buf{};
    in.getline(buf.data(), buf.size() - 1);
    if(in.fail() && !in.eof())
      return false; // probably a binary file
    std::istringstream line(buf.data());

    auto command = getCueKeyword(line);

    if(command == "TRACK")
    {
      if(canAddTrack)
        success = addTrack(track, shift, prestart, totalPregap, currPregap);
      else
        success = true;

      track.start = 0;
      track.skip = 0;
      currPregap = 0;
      prestart = 0;

      line >> track.number;
      auto type = getCueKeyword(line);

      if(type == "AUDIO")
      {
        track.sectorSize = RAW_SECTOR_SIZE;
        track.mode2 = false;
      }
      else if(type == "MODE1/2048")
      {
        track.sectorSize = COOKED_SECTOR_SIZE;
        track.mode2 = false;
      }
      else if(type == "MODE1/2352")
      {
        track.sectorSize = RAW_SECTOR_SIZE;
        track.mode2 = false;
      }
      else if(type == "MODE2/2336")
      {
        track.sectorSize = 2336;
        track.mode2 = true;
      }
      else if(type == "MODE2/2352")
      {
        track.sectorSize = RAW_SECTOR_SIZE;
        track.mode2 = true;
      }
      else
      {
        success = false;
        BOOST_LOG_TRIVIAL(error) << "unhandled track type " << type;
      }

      canAddTrack = true;
    }
    else if(command == "INDEX")
    {
      int index;
      line >> index;
      size_t frame;
      success = getCueFrame(frame, line);

      if(index == 1)
        track.start = frame;
      else if(index == 0)
        prestart = frame;
      // ignore other indices
    }
    else if(command == "FILE")
    {
      if(canAddTrack)
        success = addTrack(track, shift, prestart, totalPregap, currPregap);
      else
        success = true;
      canAddTrack = false;

      auto filename = getCueString(line);
      getRealFileName(filename, cuefile.parent_path());
      auto type = getCueKeyword(line);

      track.file = nullptr;
      if(type == "BINARY" || type == "MP3")
      {
        track.file = std::make_shared<BinaryFile>(filename.c_str());
      }
      else
      {
        BOOST_LOG_TRIVIAL(error) << "unhandled file type " << type;
      }
    }
    else if(command == "PREGAP")
    {
      success = getCueFrame(currPregap, line);
    }
    else if(command == "CATALOG")
    {
      auto mcn = getCueString(line);
      success = true;
    }
    // ignored commands
    else if(command == "CDTEXTFILE" || command == "FLAGS" || command == "ISRC" || command == "PERFORMER"
            || command == "POSTGAP" || command == "REM" || command == "SONGWRITER" || command == "TITLE"
            || command.empty())
    {
      success = true;
    }
    else
    {
      success = false;
      BOOST_LOG_TRIVIAL(error) << "unhandled command " << command;
    }

    if(!success)
      return false;
  }
  // add last track
  if(!addTrack(track, shift, prestart, totalPregap, currPregap))
    return false;

  // add leadout track
  track.number++;
  track.start = 0;
  track.length = 0;
  track.file = nullptr;
  if(!addTrack(track, shift, 0, totalPregap, 0))
    return false;

  return true;
}

bool CdImage::addTrack(Track& curr, size_t& shift, size_t prestart, size_t& totalPregap, size_t currPregap)
{
  // frames between index 0(prestart) and 1(curr.start) must be skipped
  size_t skip = 0;
  if(prestart > 0)
  {
    if(prestart > curr.start)
      return false;
    skip = curr.start - prestart;
  }

  // first track (track number must be 1)
  if(m_tracks.empty())
  {
    if(curr.number != 1)
    {
      BOOST_LOG_TRIVIAL(error) << "invalid initial track number";
      return false;
    }
    curr.skip = skip * curr.sectorSize;
    curr.start += currPregap;
    totalPregap = currPregap;
    m_tracks.push_back(curr);
    return true;
  }

  Track& prev = m_tracks.back();

  // current track consumes data from the same file as the previous
  if(prev.file == curr.file)
  {
    curr.start += shift;
    prev.length = curr.start + totalPregap - prev.start - skip;
    curr.skip += prev.skip + prev.length * prev.sectorSize + skip * curr.sectorSize;
    totalPregap += currPregap;
    curr.start += totalPregap;
  }
  else
  {
    const auto tmp = prev.file->size() - prev.skip;
    prev.length = tmp / prev.sectorSize;
    if(tmp % prev.sectorSize != 0)
      prev.length++; // padding

    curr.start += prev.start + prev.length + currPregap;
    curr.skip = skip * curr.sectorSize;
    shift += prev.start + prev.length;
    totalPregap = currPregap;
  }

  // error checks
  if(curr.number <= 1)
  {
    BOOST_LOG_TRIVIAL(error) << "invalid track number";
    return false;
  }
  if(prev.number + 1 != curr.number)
  {
    BOOST_LOG_TRIVIAL(error) << "invalid track number sequence";
    return false;
  }
  if(curr.start < prev.start + prev.length)
  {
    BOOST_LOG_TRIVIAL(error) << "overlapping tracks";
    return false;
  }

  m_tracks.push_back(curr);
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
