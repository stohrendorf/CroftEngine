#include "videoplayer.h"

#include "audio/device.h"
#include "audio/streamvoice.h"
#include "avdecoder.h"
#include "converter.h"
#include "ffmpeg/stream.h"
#include "filtergraph.h"

#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <chrono>
#include <filesystem>
#include <functional>
#include <gl/pixel.h>
#include <gl/soglb_fwd.h>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>

extern "C"
{
#include <libavfilter/avfilter.h>
}

namespace video
{
void play(
  const std::filesystem::path& filename,
  audio::Device& audioDevice,
  const std::function<bool(const gslu::nn_shared<gl::TextureHandle<gl::Texture2D<gl::SRGB8>>>& textureHandle)>& onFrame)
{
  if(!is_regular_file(filename))
    BOOST_THROW_EXCEPTION(std::runtime_error("Video file not found"));

  auto decoderPtr = std::make_unique<AVDecoder>(filename.string());
  BOOST_LOG_TRIVIAL(info) << "Playing " << filename << ", estimated duration "
                          << std::chrono::duration_cast<std::chrono::seconds>(decoderPtr->getDuration()).count()
                          << " seconds";
  gsl_Assert(decoderPtr->videoStream->context->pix_fmt != AV_PIX_FMT_NONE);
  std::map<AVPixelFormat, std::shared_ptr<Converter>> converters;

  const auto decoder = decoderPtr.get();

  const auto srcSize = glm::ivec2{decoderPtr->videoStream->context->width, decoderPtr->videoStream->context->height};

  auto stream = audioDevice.createStream(
    std::move(decoderPtr), audioDevice.getSampleRate() / 30, 4, std::chrono::milliseconds{0});
  stream->setLooping(true);

  const auto streamFinisher = gsl::finally(
    [&stream, &audioDevice]()
    {
      audioDevice.removeStream(stream.get());
    });

  stream->play();
  decoder->play();
  while(!decoder->stopped)
  {
    if(const auto f = decoder->takeFrame(); f.has_value())
    {
      const auto fmt = static_cast<AVPixelFormat>(f->frame->format);
      std::shared_ptr<Converter> converter;
      if(const auto it = converters.find(fmt); it != converters.end())
      {
        converter = it->second;
      }
      else
      {
        converter = std::make_shared<Converter>(srcSize, fmt);
        converters.emplace(fmt, converter);
      }

      converter->update(*f);
      decoder->stopped |= !onFrame(gsl::not_null{converter->textureHandle});
    }
  }
}
} // namespace video
