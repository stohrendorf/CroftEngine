#pragma once

#include <filesystem>
#include <functional>
#include <gl/image.h>

namespace audio
{
class Device;
}

namespace video
{
extern void play(const std::filesystem::path& filename,
                 audio::Device& audioDevice,
                 const std::shared_ptr<gl::Image<gl::SRGBA8>>& img,
                 const std::function<bool()>& onFrame);
}
