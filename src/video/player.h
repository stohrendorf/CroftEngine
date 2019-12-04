#pragma once

#include "render/gl/image.h"

#include <filesystem>
#include <functional>

namespace audio
{
class Device;
}

namespace video
{
extern void play(const std::filesystem::path& filename,
                 audio::Device& audioDevice,
                 const std::shared_ptr<render::gl::Image<render::gl::SRGBA8>>& img,
                 const std::function<bool()>& onFrame);
}
