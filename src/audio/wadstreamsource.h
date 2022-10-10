#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>

namespace audio
{
class FfmpegSubStreamStreamSource;

extern std::unique_ptr<FfmpegSubStreamStreamSource> createWadStream(const std::filesystem::path& filename,
                                                                    size_t trackIndex);
} // namespace audio
