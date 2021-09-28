#include "videoplayer.h"

#include "audio/device.h"
#include "audio/streamsource.h"
#include "audio/streamvoice.h"
#include "avdecoder.h"
#include "converter.h"
#include "filtergraph.h"

#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <chrono>
#include <filesystem>
#include <functional>
#include <gl/pixel.h>
#include <gsl/gsl-lite.hpp>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

extern "C"
{
#include <libavfilter/avfilter.h>
}

namespace video
{
void play(const std::filesystem::path& filename,
          audio::Device& audioDevice,
          const std::function<bool(const gsl::not_null<std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>>&
                                     textureHandle)>& onFrame)
{
  if(!is_regular_file(filename))
    BOOST_THROW_EXCEPTION(std::runtime_error("Video file not found"));

  auto decoderPtr = std::make_unique<AVDecoder>(filename.string());
  BOOST_LOG_TRIVIAL(info) << "Playing " << filename << ", estimated duration "
                          << std::chrono::duration_cast<std::chrono::seconds>(decoderPtr->getDuration()).count()
                          << " seconds";
  const auto decoder = decoderPtr.get();
  Expects(decoder->filterGraph.graph->sink_links_count == 1);
  Converter converter{decoder->filterGraph.graph->sink_links[0]};
  decoderPtr->fillQueues();

  auto stream
    = audioDevice.createStream(std::move(decoderPtr), decoder->audioFrameSize, 30, std::chrono::milliseconds{0});
  stream->setLooping(true);
  stream->play();

  const auto streamFinisher = gsl::finally([&stream, &audioDevice]() { audioDevice.removeStream(stream.get()); });

  while(!decoder->stopped)
  {
    if(const auto f = decoder->takeFrame())
    {
      converter.update(*f);
      decoder->stopped |= !onFrame(gsl::not_null{converter.textureHandle});
    }
  }
}
} // namespace video
