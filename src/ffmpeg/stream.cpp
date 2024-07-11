#include "stream.h"

#include <boost/throw_exception.hpp>
#include <stdexcept>
#include <string>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavcodec/version.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/dict.h>
}

namespace ffmpeg
{
Stream::Stream(AVFormatContext* fmtContext, AVMediaType type, bool rplFakeAudioHack)
    : index{av_find_best_stream(fmtContext, type, -1, -1, nullptr, 0)}
{
  if(index < 0)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Could not find stream in input file"));
  }

  stream = fmtContext->streams[index];
  if(rplFakeAudioHack)
  {
    // https://trac.ffmpeg.org/ticket/7859
#if LIBAVCODEC_VERSION_MAJOR >= 59
    if(stream->codecpar->codec_id == AV_CODEC_ID_PCM_VIDC)
#else
    if(stream->codecpar->codec_id == AV_CODEC_ID_NONE)
#endif
      stream->codecpar->codec_id = AV_CODEC_ID_PCM_U8;
  }

  const auto decoder = avcodec_find_decoder(stream->codecpar->codec_id);
  if(decoder == nullptr)
  {
    BOOST_THROW_EXCEPTION(
      std::runtime_error(std::string{"Failed to find codec: "} + avcodec_get_name(stream->codecpar->codec_id)));
  }

  context = avcodec_alloc_context3(decoder);
  if(context == nullptr)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to allocate the codec context"));
  }

  if(avcodec_parameters_to_context(context, stream->codecpar) < 0)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to copy codec parameters to decoder context"));
  }

  AVDictionary* opts = nullptr;
  av_dict_set(&opts, "refcounted_frames", "0", 0);
  if(avcodec_open2(context, decoder, &opts) < 0)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to open codec"));
  }
}

Stream::~Stream()
{
  avcodec_free_context(&context);
}
} // namespace ffmpeg
