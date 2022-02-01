#pragma once

#include <filesystem>
#include <functional>
#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
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
       const std::function<bool(const gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>& textureHandle)>&
         onFrame);
}
