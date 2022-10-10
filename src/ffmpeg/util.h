#pragma once

#include <chrono>
#include <cstdint>

extern "C"
{
#include <libavutil/rational.h>
}

namespace ffmpeg
{
template<typename T>
constexpr T toDuration(int64_t ts, const AVRational& timeBase)
{
  using period = typename T::period;
  return T{ts * (timeBase.num * period::den) / (timeBase.den * period::num)};
}

template<typename T>
constexpr int64_t fromDuration(const T& ts, const AVRational& timeBase)
{
  using period = typename T::period;
  return ts.count() * (timeBase.den * period::num) / (timeBase.num * period::den);
}
} // namespace ffmpeg
