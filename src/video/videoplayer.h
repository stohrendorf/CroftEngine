#pragma once

#include <filesystem>
#include <functional>
#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <memory>

namespace audio
{
class Device;
}

namespace video
{
extern void
  play(const std::filesystem::path& filename,
       audio::Device& audioDevice,
       const std::function<bool(const std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>& textureHandle)>&
         onFrame);
}
