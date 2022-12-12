#pragma once

#include <filesystem>
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

namespace cue
{
struct TrackCommand
{
  size_t index = 0;
  bool audio = false;
  bool mode2 = false;
  std::streamsize sectorSize = 0;
};
struct FileCommand
{
  std::string filename;
  std::string type;
};
struct IndexCommand
{
  std::size_t index = 0;
  std::size_t frame = 0;
};
struct Track
{
  size_t index = 0;
  bool audio = false;
  bool mode2xa = false;
  std::streamsize sectorSize = 0;
  size_t start = 0;
  size_t pregapStart = 0;
  std::filesystem::path filepath{};
  std::string filetype{};
  std::size_t pregap = 0;
};

extern std::optional<TrackCommand> parseTrack(const std::string& line);
extern std::optional<FileCommand> parseFile(const std::string& line);
extern std::optional<size_t> parseTime(const std::string& time);
extern std::optional<IndexCommand> parseIndex(const std::string& line);
extern std::optional<size_t> parsePregap(const std::string& line);
extern bool isIrrelevantCommand(const std::string& line);
extern std::vector<Track> readCueSheet(const std::filesystem::path& filename);
} // namespace cue
