#pragma once

#include <array>
#include <cstdint>
#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <memory>

struct AVFilterLink;
struct SwsContext;

namespace video
{
struct AVFramePtr;

struct Converter final
{
  AVFilterLink* filter;
  SwsContext* context;
  std::array<uint8_t*, 4> dstVideoData{nullptr};
  std::array<int, 4> dstVideoLinesize{0};
  gsl::not_null<std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>> textureHandle;

  explicit Converter(AVFilterLink* filter);

  ~Converter();

  void update(const AVFramePtr& videoFrame);
};
} // namespace video
