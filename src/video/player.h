#pragma once

#include "render/gl/image.h"

#include <boost/filesystem/path.hpp>

namespace audio
{
class Device;
}

namespace video
{
extern void play(const boost::filesystem::path& filename,
                 audio::Device& audioDevice,
                 const std::shared_ptr<render::gl::Image<render::gl::RGBA8>>& img,
                 const std::function<bool()>& onFrame
);
}
