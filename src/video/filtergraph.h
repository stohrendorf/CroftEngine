#pragma once

struct AVFilterContext;
struct AVFilterInOut;
struct AVFilterGraph;

namespace video
{
struct Stream;

struct FilterGraph final
{
  AVFilterContext* input = nullptr;
  AVFilterContext* output = nullptr;
  AVFilterGraph* graph;
  AVFilterInOut* outputs;
  AVFilterInOut* inputs;

  explicit FilterGraph();

  ~FilterGraph();

  void init(const Stream& stream);
};
} // namespace video