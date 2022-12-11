#include "cueparser.h"

#include <boost/log/trivial.hpp>
#include <regex>

namespace cue
{
std::optional<TrackCommand> parseTrack(const std::string& line)
{
  static const std::regex audioRe{R"(\s*TRACK\s+(\d+)\s+AUDIO\s*)", std::regex_constants::syntax_option_type::icase};
  if(std::smatch m; std::regex_match(line, m, audioRe))
  {
    return TrackCommand{std::stoul(m[1]), true, false, 2352};
  }

  static const std::regex dataRe{R"(\s*TRACK\s+(\d+)\s+MODE([12])/(\d+)\s*)",
                                 std::regex_constants::syntax_option_type::icase};
  if(std::smatch m; std::regex_match(line, m, dataRe))
  {
    return TrackCommand{std::stoul(m[1]), false, m[2] == "2", std::stoul(m[3])};
  }

  BOOST_LOG_TRIVIAL(trace) << "failed to parse TRACK command " << line;

  return std::nullopt;
}

std::optional<FileCommand> parseFile(const std::string& line)
{
  static const std::regex quotedRe{"\\s*FILE\\s+\"(.+)\"\\s+(.+)\\s*", std::regex_constants::syntax_option_type::icase};
  if(std::smatch m; std::regex_match(line, m, quotedRe))
  {
    return FileCommand{m[1], m[2]};
  }
  static const std::regex unquotedRe{R"(\s*FILE\s+(.+)\s+(.+)\s*)", std::regex_constants::syntax_option_type::icase};
  if(std::smatch m; std::regex_match(line, m, unquotedRe))
  {
    return FileCommand{m[1], m[2]};
  }

  BOOST_LOG_TRIVIAL(trace) << "failed to parse FILE command " << line;

  return std::nullopt;
}

std::optional<size_t> parseTime(const std::string& time)
{
  static const std::regex timeRe{R"((\d+):(\d+):(\d+))"};
  if(std::smatch m; std::regex_match(time, m, timeRe))
  {
    return (std::stoul(m[1]) * 60u + std::stoul(m[2])) * 75u + std::stoul(m[3]);
  }

  BOOST_LOG_TRIVIAL(trace) << "Failed to parse time " << time;

  return std::nullopt;
}

std::optional<IndexCommand> parseIndex(const std::string& line)
{
  static const std::regex indexRe{R"(\s*INDEX\s+(\d+)\s+(.+?)\s*)", std::regex_constants::syntax_option_type::icase};
  if(std::smatch m; std::regex_match(line, m, indexRe))
  {
    if(auto frame = parseTime(m[2]); frame.has_value())
      return IndexCommand{std::stoul(m[1]), *frame};
  }

  BOOST_LOG_TRIVIAL(trace) << "failed to parse INDEX command " << line;

  return std::nullopt;
}

std::optional<size_t> parsePregap(const std::string& line)
{
  static const std::regex pregapRe{R"(\s*INDEX\s+(.+?)\s*)", std::regex_constants::syntax_option_type::icase};
  if(std::smatch m; std::regex_match(line, m, pregapRe))
  {
    if(auto frame = parseTime(m[1]); frame.has_value())
      return frame;
  }

  BOOST_LOG_TRIVIAL(trace) << "failed to parse PREGAP command " << line;

  return std::nullopt;
}

bool isIrrelevantCommand(const std::string& line)
{
  static const std::regex ignoreRe{
    R"(\s*((CATALOG|CDTEXTFILE|FLAGS|ISRC|PERFORMER|POSTGAP|REM|SONGWRITER|TITLE)\s+.+)?\s*)",
    std::regex_constants::syntax_option_type::icase};
  return std::regex_match(line, ignoreRe);
}
} // namespace cue
