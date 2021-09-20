#pragma once

#include "core.h"
#include "streamsource.h"
#include "video/ffmpegstreamsource.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>

namespace audio
{
extern std::unique_ptr<video::FfmpegSubStreamStreamSource> createWadStream(const std::filesystem::path& filename,
                                                                           size_t trackIndex);
} // namespace audio
