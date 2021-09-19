#include "avframeptr.h"

#include <gsl/gsl-lite.hpp>
#include <utility>

extern "C"
{
#include <libavutil/frame.h>
}

namespace video
{
AVFramePtr::AVFramePtr()
    : frame{av_frame_alloc()}
{
  Expects(frame != nullptr);
}

AVFramePtr::AVFramePtr(AVFramePtr&& rhs) noexcept
    : frame{std::exchange(rhs.frame, nullptr)}
{
}

AVFramePtr& AVFramePtr::operator=(AVFramePtr&& rhs) noexcept
{
  av_frame_free(&frame);
  frame = std::exchange(rhs.frame, nullptr);
  return *this;
}

AVFrame* AVFramePtr::release()
{
  return std::exchange(frame, nullptr);
}

AVFramePtr::~AVFramePtr()
{
  av_frame_free(&frame);
}
} // namespace video
