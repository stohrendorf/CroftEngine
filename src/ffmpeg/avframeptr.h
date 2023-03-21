#pragma once

struct AVFrame;

namespace ffmpeg
{
struct AVFramePtr final
{
  AVFrame* frame{};

  explicit AVFramePtr();
  AVFramePtr(AVFramePtr&& rhs) noexcept;
  ~AVFramePtr();

  AVFramePtr& operator=(AVFramePtr&& rhs) noexcept;

  AVFrame* release() noexcept;
};
} // namespace ffmpeg
