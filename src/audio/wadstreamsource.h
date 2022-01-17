#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>

namespace video
{
class FfmpegSubStreamStreamSource;
}

namespace audio
{
extern std::unique_ptr<video::FfmpegSubStreamStreamSource> createWadStream(const std::filesystem::path& filename,
                                                                           size_t trackIndex);
} // namespace audio
