#pragma once

#include <array>
#include <cstdint>
#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <glm/vec2.hpp>
#include <gslu.h>

extern "C"
{
#include <libavutil/avutil.h>
}

struct SwsContext;

namespace ffmpeg
{
struct AVFramePtr;
}

namespace video
{
struct Converter final
{
  glm::ivec2 size;
  SwsContext* context;
  std::array<uint8_t*, 4> dstVideoData{nullptr};
  std::array<int, 4> dstVideoLinesize{0};
  gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::SRGB8>>> textureHandle;

  explicit Converter(const glm::ivec2& size, AVPixelFormat srcFormat);

  ~Converter();

  void update(const ffmpeg::AVFramePtr& videoFrame);
};
} // namespace video
