#pragma once

#include <filesystem>
#include <functional>
#include <gl/pixel.h>
#include <gl/soglb_fwd.h>

namespace SoLoud
{
class Soloud;
}

namespace video
{
extern void
  play(const std::filesystem::path& filename,
       SoLoud::Soloud& soLoud,
       const std::function<bool(const std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>& textureHandle)>&
         onFrame);
}
