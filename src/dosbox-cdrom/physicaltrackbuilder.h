#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>
#include <utility>
#include <vector>

namespace cue
{
struct Track;
}

namespace image
{
class BinaryFile;

struct Track
{
  size_t startSector = 0;
  size_t sectorSize = 0;
  bool mode2xa = false;
  std::shared_ptr<BinaryFile> file;
  size_t totalSectors = 0;
  size_t fileOffset = 0;

  [[nodiscard]] auto getEndSector() const noexcept
  {
    return startSector + totalSectors;
  }
};

class PhysicalTrackBuilder final
{
public:
  explicit PhysicalTrackBuilder(const std::filesystem::path& cueFilepath);
  [[nodiscard]] auto getTracks() &&
  {
    return std::move(m_tracks);
  }

private:
  bool addTrack(const cue::Track& cueTrack, const std::shared_ptr<BinaryFile>& file);

  std::vector<Track> m_tracks;
  size_t m_discSectorStart = 0;
  size_t m_totalPregap = 0;
};
} // namespace image
