#include "cueparser.h"

#include <boost/log/trivial.hpp>
#include <fstream>
#include <gsl/gsl-lite.hpp>
#include <regex>

namespace cue
{
/*
 * From https://web.archive.org/web/20070614044112/http://www.goldenhawk.com/download/cdrwin.pdf, the available commands
 * are CATALOG, CDTEXTFILE, FILE, FLAGS, INDEX, ISRC, PERFORMER, POSTGAP, PREGAP, REM, SONGWRITER, TITLE, and TRACK.
 *
 * CATALOG and CDTEXTFILE can appear only once, anywhere in the file.
 * FILE has parameters $filename and $filetype. $filetype can be BINARY, MOTOROLA, AIFF, WAVE, or MP3. Must appear
 * before any other command.
 * FLAGS must appear after TRACK, and before INDEX. Only once per TRACK.
 * INDEX has parameters $number and $start. First INDEX must have $number 0 or 1, and the first INDEX of a TRACK must
 * have $start=00:00:00. Following INDEX commands must have sequential $number values. $number 0 is the pregap starting
 * time, $number 1 is the start time of the TRACK data.
 * ISRC must be after TRACK and before INDEX.
 * PERFORMER is global if before any TRACK command, otherwise it's for the current TRACK.
 * POSTGAP has a $length parameter in mm:ss:ff format. Only once per TRACK, must come after all INDEX commands.
 * PREGAP has a $length parameter in mm:ss:ff format. Only once per TRACK, must appear after TRACK, and before INDEX.
 * REM can appear anywhere, arbitrary times.
 * SONGWRITER is global if before any TRACK command, otherwise it's for the current TRACK.
 * TITLE is global if before any TRACK command, otherwise it's for the current TRACK.
 * TRACK has paremeters $number and $datatype. $datatype can be AUDIO, CDG, MODE1/2048, MODE1/2352, MODE2/2336,
 * MODE2/2352, CDI/2336, or CDI/2352. $number must be between 1 and 99 inclusive, and must be sequential. Must appear
 * at least once.
 *
 * Only considering data-defining commands, the cue sheet has the following format:
 * FILE $filename $filetype (only required before any TRACK)
 * TRACK $number $datatype (at least once, with the following commands applying to this TRACK)
 *   PREGAP $length (optional)
 *   INDEX $number $start (at least once)
 *   POSTGAP $length (optional)
 */

std::optional<TrackCommand> parseTrack(const std::string& line)
{
  static const std::regex audioRe{R"(\s*TRACK\s+(\d+)\s+AUDIO\s*)", std::regex_constants::icase};
  if(std::smatch m; std::regex_match(line, m, audioRe))
  {
    return TrackCommand{std::stoul(m[1]), true, false, 2352};
  }

  static const std::regex dataRe{R"(\s*TRACK\s+(\d+)\s+MODE([12])/(\d+)\s*)", std::regex_constants::icase};
  if(std::smatch m; std::regex_match(line, m, dataRe))
  {
    return TrackCommand{std::stoul(m[1]), false, m[2] == "2", std::stoul(m[3])};
  }

  return std::nullopt;
}

std::optional<FileCommand> parseFile(const std::string& line)
{
  static const std::regex quotedRe{"\\s*FILE\\s+\"(.+)\"\\s+(.+)\\s*", std::regex_constants::icase};
  if(std::smatch m; std::regex_match(line, m, quotedRe))
  {
    return FileCommand{m[1], m[2]};
  }
  static const std::regex unquotedRe{R"(\s*FILE\s+(.+)\s+(.+)\s*)", std::regex_constants::icase};
  if(std::smatch m; std::regex_match(line, m, unquotedRe))
  {
    return FileCommand{m[1], m[2]};
  }

  return std::nullopt;
}

std::optional<size_t> parseTime(const std::string& time)
{
  static const std::regex timeRe{R"((\d+):(\d+):(\d+))"};
  if(std::smatch m; std::regex_match(time, m, timeRe))
  {
    return (std::stoul(m[1]) * 60u + std::stoul(m[2])) * 75u + std::stoul(m[3]);
  }

  return std::nullopt;
}

std::optional<IndexCommand> parseIndex(const std::string& line)
{
  static const std::regex indexRe{R"(\s*INDEX\s+0*(\d+)\s+(.+?)\s*)", std::regex_constants::icase};
  if(std::smatch m; std::regex_match(line, m, indexRe))
  {
    if(auto frame = parseTime(m[2]); frame.has_value())
      return IndexCommand{std::stoul(m[1]), *frame};
  }

  return std::nullopt;
}

std::optional<size_t> parsePregap(const std::string& line)
{
  static const std::regex pregapRe{R"(\s*INDEX\s+(.+?)\s*)", std::regex_constants::icase};
  if(std::smatch m; std::regex_match(line, m, pregapRe))
  {
    if(auto frame = parseTime(m[1]); frame.has_value())
      return frame;
  }

  return std::nullopt;
}

bool isIrrelevantCommand(const std::string& line)
{
  static const std::regex ignoreRe{
    R"(\s*((CATALOG|CDTEXTFILE|FLAGS|ISRC|PERFORMER|POSTGAP|REM|SONGWRITER|TITLE)\s+.+)?\s*)",
    std::regex_constants::icase};
  return std::regex_match(line, ignoreRe);
}

std::vector<Track> readCueSheet(const std::filesystem::path& filename)
{
  std::ifstream cueSheet;
  cueSheet.open(filename, std::ios::in);
  if(cueSheet.fail())
  {
    BOOST_LOG_TRIVIAL(error) << "could not open " << filename;
    return {};
  }

  std::vector<Track> result;
  std::optional<Track> track;
  std::filesystem::path currentFile;
  std::string currentFiletype;

  auto addTrack = [&result, &track, &currentFile, &currentFiletype]()
  {
    if(!track.has_value())
      return;

    gsl_Assert(!currentFile.empty());
    gsl_Assert(!currentFiletype.empty());
    gsl_Assert(track.has_value());
    result.emplace_back(*track);
  };

  for(std::string line; std::getline(cueSheet, line);)
  {
    if(const auto trackCmd = cue::parseTrack(line); trackCmd.has_value())
    {
      BOOST_LOG_TRIVIAL(debug) << "Track command: index=" << trackCmd->index << ", mode2=" << trackCmd->mode2
                               << ", sectorSize=" << trackCmd->sectorSize << ", audio=" << trackCmd->audio;

      addTrack();
      track = Track{trackCmd->index, trackCmd->audio, trackCmd->mode2, trackCmd->sectorSize};
      track->filepath = currentFile;
      track->filetype = currentFiletype;
    }
    else if(const auto indexCmd = cue::parseIndex(line); indexCmd.has_value())
    {
      BOOST_LOG_TRIVIAL(debug) << "Index command: index=" << indexCmd->index << ", frame=" << indexCmd->frame;
      gsl_Assert(track.has_value());

      if(indexCmd->index == 0)
      {
        // pregap start time
        track->pregapStart = indexCmd->frame;
      }
      else
      {
        track->start = indexCmd->frame;
      }
    }
    else if(const auto fileCmd = cue::parseFile(line); fileCmd.has_value())
    {
      BOOST_LOG_TRIVIAL(debug) << "File command: filename=" << fileCmd->filename << ", type=" << fileCmd->type;

      currentFile = filename.parent_path() / fileCmd->filename;
      currentFiletype = fileCmd->type;
    }
    else if(const auto pregapCmd = cue::parsePregap(line); pregapCmd.has_value())
    {
      BOOST_LOG_TRIVIAL(debug) << "Pregap command: pregap=" << *pregapCmd;
      gsl_Assert(track.has_value());

      track->pregap = *pregapCmd;
    }
    else if(cue::isIrrelevantCommand(line))
    {
      BOOST_LOG_TRIVIAL(debug) << "Ignored line: " << line;
    }
    else
    {
      BOOST_LOG_TRIVIAL(error) << "Unhandled line " << line;
      return {};
    }
  }

  addTrack();

  return result;
}
} // namespace cue
