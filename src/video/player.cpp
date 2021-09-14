#include "player.h"

#include "audio/core.h"
#include "audio/device.h"
#include "audio/streamsource.h"
#include "audio/streamvoice.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <functional>
#include <gl/pixel.h>
#include <gl/sampler.h>
#include <gl/texture2d.h>
#include <gl/texturehandle.h>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <iterator>
#include <mutex>
#include <optional>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

namespace video
{
inline std::string getAvError(int err)
{
  std::vector<char> tmp(1024, 0);
  if(av_strerror(err, tmp.data(), tmp.size()) < 0)
    return "Unknown error " + std::to_string(err);

  return tmp.data();
}

struct Stream final
{
  const int index;
  AVCodecContext* context = nullptr;
  AVStream* stream = nullptr;

  Stream(AVFormatContext* fmtContext, AVMediaType type)
      : index{av_find_best_stream(fmtContext, type, -1, -1, nullptr, 0)}
  {
    if(index < 0)
    {
      BOOST_THROW_EXCEPTION(std::runtime_error("Could not find stream in input file"));
    }

    stream = fmtContext->streams[index];
    // https://trac.ffmpeg.org/ticket/7859
#if LIBAVCODEC_VERSION_MAJOR >= 59
    if(stream->codecpar->codec_id == AV_CODEC_ID_PCM_VIDC)
#else
    if(stream->codecpar->codec_id == AV_CODEC_ID_NONE)
#endif
      stream->codecpar->codec_id = AV_CODEC_ID_PCM_U8;

    const auto decoder = avcodec_find_decoder(stream->codecpar->codec_id);
    if(decoder == nullptr)
    {
      BOOST_THROW_EXCEPTION(std::runtime_error("Failed to find codec"));
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

  ~Stream()
  {
    avcodec_free_context(&context);
  }
};

struct AVFramePtr
{
  AVFrame* frame{};

  AVFramePtr()
      : frame{av_frame_alloc()}
  {
    Expects(frame != nullptr);
  }

  AVFramePtr(AVFramePtr&& rhs) noexcept
      : frame{std::exchange(rhs.frame, nullptr)}
  {
  }

  AVFramePtr& operator=(AVFramePtr&& rhs) noexcept
  {
    av_frame_free(&frame);
    frame = std::exchange(rhs.frame, nullptr);
    return *this;
  }

  auto release()
  {
    return std::exchange(frame, nullptr);
  }

  ~AVFramePtr()
  {
    av_frame_free(&frame);
  }
};

struct FilterGraph
{
  AVFilterContext* input = nullptr;
  AVFilterContext* output = nullptr;
  AVFilterGraph* graph = avfilter_graph_alloc();
  AVFilterInOut* outputs = avfilter_inout_alloc();
  AVFilterInOut* inputs = avfilter_inout_alloc();

  ~FilterGraph()
  {
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
    avfilter_graph_free(&graph);
  }

  void init(const Stream& stream)
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

    if(avfilter_graph_create_filter(
         &input, avfilter_get_by_name("buffer"), "in", filterGraphArgs.data(), nullptr, graph)
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
    if(avfilter_graph_parse_ptr(graph, "gblur, noise=alls=10:allf=t+u", &inputs, &outputs, nullptr) < 0)
      BOOST_THROW_EXCEPTION(std::runtime_error("Failed to initialize filter graph"));

    if(avfilter_graph_config(graph, nullptr) < 0)
      BOOST_THROW_EXCEPTION(std::runtime_error("Failed to configure filter graph"));
  }
};

struct AVDecoder final : public audio::AbstractStreamSource
{
  AVFormatContext* fmtContext = nullptr;
  AVFramePtr audioFrame;
  std::unique_ptr<Stream> audioStream;
  std::unique_ptr<Stream> videoStream;
  SwrContext* swrContext = nullptr;
  FilterGraph filterGraph;

  ~AVDecoder() override
  {
    swr_free(&swrContext);
    avformat_close_input(&fmtContext);
  }

  explicit AVDecoder(const std::string& filename)
  {
    if(avformat_open_input(&fmtContext, filename.c_str(), nullptr, nullptr) < 0)
    {
      BOOST_THROW_EXCEPTION(std::runtime_error("Could not open source file"));
    }

    if(avformat_find_stream_info(fmtContext, nullptr) < 0)
    {
      BOOST_THROW_EXCEPTION(std::runtime_error("Could not find stream information"));
    }

    videoStream = std::make_unique<Stream>(fmtContext, AVMEDIA_TYPE_VIDEO);

    audioStream = std::make_unique<Stream>(fmtContext, AVMEDIA_TYPE_AUDIO);

#ifndef NDEBUG
    av_dump_format(fmtContext, 0, filename.c_str(), 0);
#endif

    Expects(audioStream->context->channels == 1 || audioStream->context->channels == 2);

    swrContext = swr_alloc_set_opts(nullptr,
                                    // NOLINTNEXTLINE(hicpp-signed-bitwise)
                                    audioStream->context->channels == 1 ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO,
                                    AV_SAMPLE_FMT_S16,
                                    audioStream->context->sample_rate,
                                    // NOLINTNEXTLINE(hicpp-signed-bitwise)
                                    audioStream->context->channels == 1 ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO,
                                    audioStream->context->sample_fmt,
                                    audioStream->context->sample_rate,
                                    0,
                                    nullptr);
    if(swrContext == nullptr)
    {
      BOOST_THROW_EXCEPTION(std::runtime_error("Could not allocate resampler context"));
    }

    if(swr_init(swrContext) < 0)
    {
      BOOST_THROW_EXCEPTION(std::runtime_error("Failed to initialize the resampling context"));
    }

    filterGraph.init(*videoStream);

    Expects(videoStream->stream->time_base.den != 0);
    audioFrameSize
      = audioStream->context->sample_rate * videoStream->stream->time_base.num / videoStream->stream->time_base.den;

    Expects(av_new_packet(&packet, 0) == 0);
    fillQueues();
  }

  bool stopped = false;
  AVPacket packet{};

  void fillQueues()
  {
    {
      std::unique_lock lock(imgQueueMutex);
      if(audioQueue.size() >= QueueLimit || imgQueue.size() >= QueueLimit)
      {
        return;
      }
    }

    int err;
    while((err = av_read_frame(fmtContext, &packet)) == 0)
    {
      decodePacket();
      av_packet_unref(&packet);

      std::unique_lock lock(imgQueueMutex);
      if(audioQueue.size() >= QueueLimit || imgQueue.size() >= QueueLimit)
      {
        break;
      }
    }
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    if(err != 0 && err != AVERROR_EOF)
    {
      BOOST_LOG_TRIVIAL(warning) << "fillQueues done: " << getAvError(err) << "; audio=" << audioQueue.size()
                                 << ", video=" << imgQueue.size();
    }
  }

  std::queue<AVFramePtr> imgQueue;
  std::queue<std::vector<int16_t>> audioQueue;
  mutable std::mutex imgQueueMutex;
  std::condition_variable frameReadyCondition;
  bool frameReady = false;

  std::optional<AVFramePtr> takeFrame()
  {
    std::unique_lock lock{imgQueueMutex};
    while(!frameReady)
      frameReadyCondition.wait(lock);
    frameReady = false;

    std::optional<AVFramePtr> img;
    if(!imgQueue.empty())
    {
      img = std::move(imgQueue.front());
      imgQueue.pop();
      const auto& tb = videoStream->stream->time_base;
      const auto audioTs = static_cast<double>(totalAudioFrames) / static_cast<double>(audioFrameSize);
      const auto videoTs = static_cast<double>(img->frame->pts) * tb.num / tb.den;
      if(audioTs < videoTs)
      {
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds{gsl::narrow_cast<int>((videoTs - audioTs) * 1000)});
        lock.lock();
      }
    }

    stopped = imgQueue.empty() && audioQueue.empty();

    return img;
  }

  static constexpr size_t QueueLimit = 60;

  void decodeVideoPacket()
  {
    if(const auto sendPacketErr = avcodec_send_packet(videoStream->context, &packet))
    {
      if(sendPacketErr == AVERROR(EINVAL))
      {
        BOOST_LOG_TRIVIAL(info) << "Flushing video decoder";
        avcodec_flush_buffers(videoStream->context);
      }
      else
      {
        if(sendPacketErr == AVERROR(EAGAIN))
          BOOST_LOG_TRIVIAL(error) << "Frames still present in video decoder";
        else if(sendPacketErr == AVERROR(ENOMEM))
          BOOST_LOG_TRIVIAL(error) << "Failed to add packet to video decoder queue";
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        else if(sendPacketErr == AVERROR_EOF)
          BOOST_LOG_TRIVIAL(error) << "Video decoder already flushed";

        BOOST_LOG_TRIVIAL(error) << "Failed to send packet to video decoder: " << getAvError(sendPacketErr);
        BOOST_THROW_EXCEPTION(std::runtime_error("Failed to send packet to video decoder"));
      }
    }

    AVFramePtr videoFrame;
    int err;
    while((err = avcodec_receive_frame(videoStream->context, videoFrame.frame)) == 0)
    {
      if(const auto addFrameErr = av_buffersrc_add_frame(filterGraph.input, videoFrame.release()))
      {
        BOOST_LOG_TRIVIAL(error) << "Error while feeding the filtergraph: " << getAvError(addFrameErr);
        BOOST_THROW_EXCEPTION(std::runtime_error("Error while feeding the filtergraph"));
      }
      videoFrame = AVFramePtr();

      while(true)
      {
        AVFramePtr filteredFrame;
        const auto ret = av_buffersink_get_frame(filterGraph.output, filteredFrame.frame);
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
          break;
        if(ret < 0)
        {
          BOOST_LOG_TRIVIAL(error) << "Filter error: " << getAvError(ret);
          BOOST_THROW_EXCEPTION(std::runtime_error("Filter error"));
        }

        std::unique_lock lock(imgQueueMutex);
        imgQueue.push(std::move(filteredFrame));
      }
    }
    if(err != AVERROR(EAGAIN))
      BOOST_LOG_TRIVIAL(info) << "Video stream chunk decoded: " << getAvError(err);
  }

  void decodeAudioPacket()
  {
    if(const auto err = avcodec_send_packet(audioStream->context, &packet))
    {
      if(err == AVERROR(EINVAL))
      {
        BOOST_LOG_TRIVIAL(info) << "Flushing audio decoder";
        avcodec_flush_buffers(audioStream->context);
      }
      else
      {
        if(err == AVERROR(EAGAIN))
          BOOST_LOG_TRIVIAL(error) << "Frames still present in audio decoder";
        else if(err == AVERROR(ENOMEM))
          BOOST_LOG_TRIVIAL(error) << "Failed to add packet to audio decoder queue";
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        else if(err == AVERROR_EOF)
          BOOST_LOG_TRIVIAL(error) << "Audio decoder already flushed";

        BOOST_LOG_TRIVIAL(error) << "Failed to send packet to audio decoder: " << getAvError(err);
        BOOST_THROW_EXCEPTION(std::runtime_error("Failed to send packet to audio decoder"));
      }
    }

    int err;
    while((err = avcodec_receive_frame(audioStream->context, audioFrame.frame)) == 0)
    {
      const auto outSamples = swr_get_out_samples(swrContext, audioFrame.frame->nb_samples);
      if(outSamples < 0)
      {
        BOOST_LOG_TRIVIAL(error) << "Failed to receive resampled audio data";
        BOOST_THROW_EXCEPTION(std::runtime_error("Failed to receive resampled audio data"));
      }

      std::vector<int16_t> audio(gsl::narrow_cast<size_t>(outSamples) * 2u, 0);
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      auto* audioData = reinterpret_cast<uint8_t*>(audio.data());

      const auto framesDecoded = swr_convert(swrContext,
                                             &audioData,
                                             outSamples,
                                             // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
                                             const_cast<const uint8_t**>(audioFrame.frame->data),
                                             audioFrame.frame->nb_samples);
      if(framesDecoded < 0)
      {
        BOOST_THROW_EXCEPTION(std::runtime_error("Error while converting"));
      }

      // cppcheck-suppress invalidFunctionArg
      audio.resize(gsl::narrow_cast<size_t>(framesDecoded) * 2u);

      audioQueue.push(std::move(audio));
    }
    if(err != AVERROR(EAGAIN))
      BOOST_LOG_TRIVIAL(info) << "Audio stream chunk decoded: " << getAvError(err);
  }

  void decodePacket()
  {
    if(packet.stream_index == videoStream->index)
    {
      decodeVideoPacket();
    }
    else if(packet.stream_index == audioStream->index)
    {
      decodeAudioPacket();
    }
  }

  size_t readStereo(int16_t* buffer, size_t bufferSize, bool /*looping*/) override
  {
    fillQueues();

    size_t written = 0;
    while(bufferSize != 0 && !audioQueue.empty())
    {
      auto& src = audioQueue.front();
      const auto frames = std::min(static_cast<size_t>(bufferSize), src.size() / 2);

      Expects(bufferSize >= frames);

      std::copy_n(src.data(), 2 * frames, buffer);
      bufferSize -= frames;
      written += frames;
      buffer += 2 * frames;

      src.erase(src.begin(), std::next(src.begin(), 2 * frames));
      if(src.empty())
      {
        audioQueue.pop();
      }
    }

    if(written == 0)
    {
      std::unique_lock lock{imgQueueMutex};
      if(!imgQueue.empty())
      {
        // audio ended prematurely - pad with zero audio data until all video frames are consumed
        written = bufferSize;
      }
    }

    totalAudioFrames += written;
    audioFrameOffset += written;
    Expects(audioFrameSize > 0);
    while(audioFrameOffset >= audioFrameSize)
    {
      audioFrameOffset -= audioFrameSize;
      std::unique_lock lock{imgQueueMutex};
      frameReady = true;
      frameReadyCondition.notify_one();
    }

    if(written == 0)
    {
      std::unique_lock lock{imgQueueMutex};
      frameReady = true;
      frameReadyCondition.notify_one();
    }

    std::fill_n(buffer, 2 * bufferSize, int16_t{0});
    return written;
  }

  int getSampleRate() const override
  {
    return audioStream->context->sample_rate;
  }

  size_t audioFrameSize = 0;
  size_t audioFrameOffset = 0;
  std::atomic<size_t> totalAudioFrames = 0;

  std::chrono::milliseconds getPosition() const override
  {
    return std::chrono::milliseconds{0};
  }

  void seek(const std::chrono::milliseconds& /*position*/) override
  {
  }

  audio::Clock::duration getDuration() const override
  {
    using period = audio::Clock::duration::period;
    return audio::Clock::duration{videoStream->stream->duration * videoStream->stream->time_base.num * period::den
                                  / (videoStream->stream->time_base.den * period::num)};
  }
};

struct Converter
{
  static constexpr auto OutputPixFmt = AV_PIX_FMT_RGBA;

  AVFilterLink* filter;
  SwsContext* context;
  std::array<uint8_t*, 4> dstVideoData{nullptr};
  std::array<int, 4> dstVideoLinesize{0};
  std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>> textureHandle{nullptr};

  explicit Converter(AVFilterLink* filter)
      : filter{filter}
      , context{sws_getContext(filter->w,
                               filter->h,
                               static_cast<AVPixelFormat>(filter->format),
                               filter->w,
                               filter->h,
                               OutputPixFmt,
                               SWS_FAST_BILINEAR,
                               nullptr,
                               nullptr,
                               nullptr)}
  {
    auto texture = std::make_shared<gl::Texture2D<gl::SRGBA8>>(glm::ivec2{filter->w, filter->h}, "video");
    auto sampler = std::make_unique<gl::Sampler>("video");
    sampler->set(gl::api::TextureMagFilter::Linear);
    textureHandle
      = std::make_shared<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>(std::move(texture), std::move(sampler));

    if(context == nullptr)
    {
      BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create SWS context"));
    }
  }

  ~Converter()
  {
    sws_freeContext(context);
    av_freep(dstVideoData.data());
  }

  void update(const AVFramePtr& videoFrame)
  {
    Expects(videoFrame.frame->width == filter->w && videoFrame.frame->height == filter->h);
    Expects((textureHandle->getTexture()->size() == glm::ivec2{filter->w, filter->h}));
    av_freep(dstVideoData.data());
    if(av_image_alloc(dstVideoData.data(), dstVideoLinesize.data(), filter->w, filter->h, OutputPixFmt, 1) < 0)
    {
      BOOST_THROW_EXCEPTION(std::runtime_error("Could not allocate raw video buffer"));
    }
    sws_scale(context,
              static_cast<const uint8_t* const*>(videoFrame.frame->data),
              videoFrame.frame->linesize,
              0,
              videoFrame.frame->height,
              dstVideoData.data(),
              dstVideoLinesize.data());

    std::vector<gl::SRGBA8> dstData;
    dstData.resize(gsl::narrow_cast<size_t>(filter->w * filter->h), {0, 0, 0, 255});

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    std::copy_n(reinterpret_cast<gl::SRGBA8*>(dstVideoData[0]), filter->w * filter->h, dstData.data());

    textureHandle->getTexture()->assign(dstData.data());
  }
};

void play(const std::filesystem::path& filename,
          audio::Device& audioDevice,
          const std::function<bool(const std::shared_ptr<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>& textureHandle)>&
            onFrame)
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
    = audioDevice.createStream(std::move(decoderPtr), decoder->audioFrameSize, 2, std::chrono::milliseconds{0});
  stream->setLooping(true);
  stream->play();

  const auto streamFinisher = gsl::finally([&stream, &audioDevice]() { audioDevice.removeStream(stream); });

  while(!decoder->stopped)
  {
    if(const auto f = decoder->takeFrame())
    {
      converter.update(*f);
      decoder->stopped |= !onFrame(converter.textureHandle);
    }
  }
}
} // namespace video
