#include "filtergraph.h"

#include "ffmpeg/stream.h"

#include <array>
#include <boost/throw_exception.hpp>
#include <cstdio>
#include <gsl/gsl-lite.hpp>
#include <stdexcept>

extern "C"
{
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
}

namespace video
{
FilterGraph::FilterGraph()
    : graph{avfilter_graph_alloc()}
    , outputs{avfilter_inout_alloc()}
    , inputs{avfilter_inout_alloc()}
{
}

FilterGraph::~FilterGraph()
{
  avfilter_inout_free(&inputs);
  avfilter_inout_free(&outputs);
  avfilter_graph_free(&graph);
}

void FilterGraph::init(const ffmpeg::Stream& stream)
{
  std::array<char, 512> filterGraphArgs{};
  snprintf(filterGraphArgs.data(),
           filterGraphArgs.size(),
           "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
           stream.stream->codecpar->width,
           stream.stream->codecpar->height,
           stream.stream->codecpar->format,
           stream.stream->time_base.num,
           stream.stream->time_base.den,
           stream.stream->codecpar->sample_aspect_ratio.num,
           stream.stream->codecpar->sample_aspect_ratio.den);
  {
    [[maybe_unused]] const auto check = gsl::ensure_z(filterGraphArgs.data(), filterGraphArgs.size());
  }

  if(avfilter_graph_create_filter(&input, avfilter_get_by_name("buffer"), "in", filterGraphArgs.data(), nullptr, graph)
     < 0)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Cannot create buffer source"));
  }

  if(avfilter_graph_create_filter(&output, avfilter_get_by_name("buffersink"), "out", nullptr, nullptr, graph) < 0)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Cannot create buffer sink"));
  }

  outputs->name = av_strdup("in");
  outputs->filter_ctx = input;
  outputs->pad_idx = 0;
  outputs->next = nullptr;
  inputs->name = av_strdup("out");
  inputs->filter_ctx = output;
  inputs->pad_idx = 0;
  inputs->next = nullptr;
  if(avfilter_graph_parse_ptr(graph, "noise=alls=10:allf=t+u", &inputs, &outputs, nullptr) < 0)
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to initialize filter graph"));

  if(avfilter_graph_config(graph, nullptr) < 0)
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to configure filter graph"));
}
} // namespace video
