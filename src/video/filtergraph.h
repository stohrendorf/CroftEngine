#pragma once

struct AVFilterContext;
struct AVFilterInOut;
struct AVFilterGraph;

namespace ffmpeg
{
struct Stream;
}

namespace video
{
struct FilterGraph final
{
  AVFilterContext* input = nullptr;
  AVFilterContext* output = nullptr;
  AVFilterGraph* graph;
  AVFilterInOut* outputs;
  AVFilterInOut* inputs;

  explicit FilterGraph();

  ~FilterGraph();

  void init(const ffmpeg::Stream& stream);
};
} // namespace video