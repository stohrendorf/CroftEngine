#pragma once

#include <cstdint> // IWYU pragma: keep

extern "C"
{
#include <libavutil/avutil.h>
}

struct AVCodecContext;
struct AVStream;
struct AVFormatContext;

namespace video
{
struct Stream final
{
  const int index;
  AVCodecContext* context = nullptr;
  AVStream* stream = nullptr;

  Stream(AVFormatContext* fmtContext, AVMediaType type, bool rplFakeAudioHack);

  ~Stream();
};
} // namespace video