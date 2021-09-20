#include "wadstreamsource.h"

#include "core.h"

#include <algorithm>
#include <array>
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <cstdio>
#include <cstring>
#include <gsl/gsl-lite.hpp>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace audio
{
// CDAUDIO.WAD step size defines CDAUDIO's header stride, on which each track
// info is placed. Also CDAUDIO count specifies static amount of tracks existing
// in CDAUDIO.WAD file. Name length specifies maximum string size for trackname.
constexpr size_t WADStride = 268;
constexpr size_t WADNameLength = 260;
constexpr size_t WADCount = 130;

std::unique_ptr<video::FfmpegSubStreamStreamSource> createWadStream(const std::filesystem::path& filename,
                                                                    const size_t trackIndex)
{
  auto wadFile = std::make_unique<std::ifstream>(filename, std::ios::in | std::ios::binary);
  BOOST_LOG_TRIVIAL(trace) << "Creating WAD stream source from " << filename << ", track " << trackIndex;

  if(!wadFile->is_open())
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open WAD file"));

  wadFile->seekg(gsl::narrow<std::ifstream::off_type>(trackIndex * WADStride), std::ios::beg);

  std::array<char, WADNameLength> trackName{};
  wadFile->read(trackName.data(), WADNameLength);

  BOOST_LOG_TRIVIAL(info) << "Loading WAD track " << trackIndex << ": " << trackName.data();

  uint32_t offset = 0;
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  wadFile->read(reinterpret_cast<char*>(&offset), 4);

  uint32_t length = 0;
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  wadFile->read(reinterpret_cast<char*>(&length), 4);

  wadFile->seekg(offset, std::ios::beg);
  return std::make_unique<video::FfmpegSubStreamStreamSource>(std::move(wadFile), offset, offset + length);
}
} // namespace audio
