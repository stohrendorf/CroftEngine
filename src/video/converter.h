#pragma once

#include <array>
#include <cstdint>
#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <gslu.h>

struct AVFilterLink;
struct SwsContext;

namespace ffmpeg
{
struct AVFramePtr;
}

namespace video
{
struct Converter final
{
  AVFilterLink* filter;
  SwsContext* context;
  std::array<uint8_t*, 4> dstVideoData{nullptr};
  std::array<int, 4> dstVideoLinesize{0};
  gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>> textureHandle;

  explicit Converter(AVFilterLink* filter);

  ~Converter();

  void update(const ffmpeg::AVFramePtr& videoFrame);
};
} // namespace video
