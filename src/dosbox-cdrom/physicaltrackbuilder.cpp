#include "physicaltrackbuilder.h"

#include "binaryfile.h"
#include "cueparser.h"

#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <gsl/gsl-lite.hpp>
#include <stdexcept>
#include <string>

namespace image
{
PhysicalTrackBuilder::PhysicalTrackBuilder(const std::filesystem::path& cueFilepath)
{
  const auto tracks = cue::readCueSheet(cueFilepath);
  std::shared_ptr<BinaryFile> file;
  for(const auto& track : tracks)
  {
    if(file == nullptr || track.filepath != file->getFilepath())
    {
      gsl_Assert(!track.filepath.empty());
      file = std::make_shared<BinaryFile>(track.filepath);
    }
    if(!addTrack(track, file))
      BOOST_THROW_EXCEPTION(std::runtime_error("could not add track from cue sheet"));
  }
}

bool PhysicalTrackBuilder::addTrack(const cue::Track& cueTrack, const std::shared_ptr<BinaryFile>& file)
{
  if(cueTrack.index != m_tracks.size() + 1)
  {
    BOOST_LOG_TRIVIAL(error) << "invalid track number";
    return false;
  }

  Track track{cueTrack.start, cueTrack.sectorSize, cueTrack.mode2xa, file};

  track.totalSectors = (track.file->size() + track.sectorSize - 1) / track.sectorSize;

  size_t inFileSector = 0;
  if(cueTrack.pregapStart > 0)
  {
    if(cueTrack.pregapStart > cueTrack.start)
    {
      BOOST_LOG_TRIVIAL(error) << "invalid pregap start";
      return false;
    }
    inFileSector = cueTrack.start - cueTrack.pregapStart;
  }

  // first track (track number must be 1)
  if(m_tracks.empty())
  {
    track.fileOffset = inFileSector * cueTrack.sectorSize;
    track.startSector += cueTrack.pregap;
    m_totalPregap = cueTrack.pregap;
    BOOST_LOG_TRIVIAL(debug) << "Initial track: startSector=" << track.startSector
                             << ", totalSectors=" << track.totalSectors << ", fileOffset=" << track.fileOffset
                             << ", sectorSize=" << track.sectorSize << ", mode2xa=" << track.mode2xa << ", file="
                             << (track.file == nullptr ? "<none>" : track.file->getFilepath().string().c_str());

    m_tracks.push_back(track);
    return true;
  }

  Track& prev = m_tracks.back();

  if(prev.file == file)
  {
    track.startSector += m_discSectorStart;
    prev.totalSectors = track.startSector - prev.startSector + m_totalPregap - inFileSector;
    track.fileOffset += prev.fileOffset + prev.totalSectors * prev.sectorSize + inFileSector * cueTrack.sectorSize;
    m_totalPregap += cueTrack.pregap;
    track.startSector += m_totalPregap;
  }
  else
  {
    track.startSector += prev.startSector + prev.totalSectors + cueTrack.pregap;
    track.fileOffset = inFileSector * cueTrack.sectorSize;
    m_discSectorStart += prev.startSector + prev.totalSectors;
    m_totalPregap = cueTrack.pregap;
  }

  // error checks
  if(track.startSector < prev.getEndSector())
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
} // namespace image
