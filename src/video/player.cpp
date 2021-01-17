#include "player.h"

#include "audio/soundengine.h"

#include <optional>
#include <soloud.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavcodec/version.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

#include <condition_variable>
#include <filesystem>
#include <functional>
#include <gsl-lite.hpp>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

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
    char filterGraphArgs[512];
    snprintf(filterGraphArgs,
             sizeof(filterGraphArgs),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             stream.stream->codecpar->width,
             stream.stream->codecpar->height,
             stream.stream->codecpar->format,
             stream.stream->time_base.num,
             stream.stream->time_base.den,
             stream.stream->codecpar->sample_aspect_ratio.num,
             stream.stream->codecpar->sample_aspect_ratio.den);

    if(avfilter_graph_create_filter(&input, avfilter_get_by_name("buffer"), "in", filterGraphArgs, nullptr, graph) < 0)
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

struct AVDecoder final : public SoLoud::AudioSource
{
  class AVDecoderAudioInstance : public SoLoud::AudioSourceInstance
  {
  private:
    gsl::not_null<AVDecoder*> m_decoder;

  public:
    explicit AVDecoderAudioInstance(gsl::not_null<AVDecoder*> decoder)
        : m_decoder{std::move(decoder)}
    {
    }

    bool hasEnded() override
    {
      return m_decoder->audioQueue.empty() || m_decoder->stopped;
    }

    unsigned int getAudio(float* buffer, unsigned int framesToRead, unsigned int /*aBufferSize*/) override
    {
      m_decoder->fillQueues();

      size_t written = 0;
      const auto stride = framesToRead;
      while(framesToRead != 0 && !m_decoder->audioQueue.empty())
      {
        auto& src = m_decoder->audioQueue.front();
        const auto frames
          = std::min(static_cast<size_t>(framesToRead), src.size() / m_decoder->audioStream->context->channels);
        auto srcPtr = src.cbegin();

        for(size_t i = 0; i < frames; ++i)
        {
          for(int c = 0; c < m_decoder->audioStream->context->channels; ++c)
          {
            // deinterlace
            BOOST_ASSERT(srcPtr != src.cend());
            buffer[c * stride] = *srcPtr++;
          }
          BOOST_ASSERT(framesToRead > 0);
          --framesToRead;
          ++written;
          ++buffer;
        }

        src.erase(src.begin(), std::next(src.begin(), m_decoder->audioStream->context->channels * frames));
        if(src.empty())
        {
          m_decoder->audioQueue.pop();
        }
      }

      //std::fill_n(buffer, framesToRead, 0);
      buffer += framesToRead;

      m_decoder->audioFrameOffset += written;
      Expects(m_decoder->audioFrameSize > 0);
      while(m_decoder->audioFrameOffset >= m_decoder->audioFrameSize)
      {
        m_decoder->audioFrameOffset -= m_decoder->audioFrameSize;
        std::unique_lock lock{m_decoder->imgQueueMutex};
        m_decoder->frameReady = true;
        m_decoder->frameReadyCondition.notify_one();
      }

      return written;
    }
  };

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
                                    audioStream->context->channels == 1 ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO,
                                    AV_SAMPLE_FMT_FLT,
                                    audioStream->context->sample_rate,
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

    av_init_packet(&packet);
    packet.data = nullptr;
    packet.size = 0;
    fillQueues();

    SoLoud::AudioSource::mBaseSamplerate = audioStream->context->sample_rate;
    SoLoud::AudioSource::mChannels = audioStream->context->channels;
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

    BOOST_LOG_TRIVIAL(trace) << "fillQueues done: " << getAvError(err) << "; audio=" << audioQueue.size()
                             << ", video=" << imgQueue.size();
  }

  std::queue<AVFramePtr> imgQueue;
  std::queue<std::vector<float>> audioQueue;
  mutable std::mutex imgQueueMutex;
  std::condition_variable frameReadyCondition;
  bool frameReady = false;

  auto takeFrame()
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
    }

    stopped = imgQueue.empty() && audioQueue.empty();

    return img;
  }

  static constexpr size_t QueueLimit = 30;

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

      std::vector<float> audio(outSamples * 2, 0);
      auto* audioData = reinterpret_cast<uint8_t*>(audio.data()); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)

      const auto framesDecoded = swr_convert(
        swrContext,
        &audioData,
        outSamples,
        const_cast<const uint8_t**>(audioFrame.frame->data), // NOLINT(cppcoreguidelines-pro-type-const-cast)
        audioFrame.frame->nb_samples);
      if(framesDecoded < 0)
      {
        BOOST_THROW_EXCEPTION(std::runtime_error("Error while converting"));
      }

      // cppcheck-suppress invalidFunctionArg
      audio.resize(framesDecoded * 2);

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

  size_t audioFrameSize = 0;
  size_t audioFrameOffset = 0;

  SoLoud::AudioSourceInstance* createInstance() override
  {
    return new AVDecoderAudioInstance(this);
  }
};

struct Scaler
{
  static constexpr auto OutputPixFmt = AV_PIX_FMT_RGBA;

  int currentSwsWidth = -1;
  int currentSwsHeight = -1;
  int scaledWidth = -1;
  int scaledHeight = -1;
  SwsContext* context = nullptr;
  AVFilterLink* filter;
  uint8_t* dstVideoData[4] = {nullptr};
  int dstVideoLinesize[4] = {0};

  explicit Scaler(AVFilterLink* filter)
      : filter{filter}
  {
  }

  ~Scaler()
  {
    sws_freeContext(context);
    av_freep(dstVideoData);
  }

  void resize(int targetWidth, int targetHeight)
  {
    if(currentSwsWidth == targetWidth && currentSwsHeight == targetHeight)
      return;

    currentSwsWidth = targetWidth;
    currentSwsHeight = targetHeight;

    const auto imgScale = std::min(float(targetWidth) / filter->w, float(targetHeight) / filter->h);
    scaledWidth = static_cast<int>(filter->w * imgScale);
    scaledHeight = static_cast<int>(filter->h * imgScale);

    sws_freeContext(context);
    context = sws_getContext(filter->w,
                             filter->h,
                             static_cast<AVPixelFormat>(filter->format),
                             scaledWidth,
                             scaledHeight,
                             OutputPixFmt,
                             SWS_LANCZOS,
                             nullptr,
                             nullptr,
                             nullptr);
    if(context == nullptr)
    {
      BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create SWS context"));
    }

    av_freep(dstVideoData);
    if(av_image_alloc(dstVideoData, dstVideoLinesize, targetWidth, targetHeight, OutputPixFmt, 1) < 0)
    {
      BOOST_THROW_EXCEPTION(std::runtime_error("Could not allocate raw video buffer"));
    }
  };

  void scale(const AVFramePtr& videoFrame, gl::Image<gl::SRGBA8>& img)
  {
    sws_scale(context,
              static_cast<const uint8_t* const*>(videoFrame.frame->data),
              videoFrame.frame->linesize,
              0,
              videoFrame.frame->height,
              dstVideoData,
              dstVideoLinesize);

    auto srcLineRaw = dstVideoData[0];
    auto dst = img.getRawData();

    Expects(img.getWidth() <= dstVideoLinesize[0] / int(sizeof(gl::SRGBA8)));

    img.fill({0, 0, 0, 255});

    Expects(img.getWidth() >= scaledWidth);
    Expects(img.getHeight() >= scaledHeight);
    const auto xOffset = static_cast<int32_t>((img.getWidth() - scaledWidth) / 2);
    const auto yOffset = static_cast<int32_t>((img.getHeight() - scaledHeight) / 2);
    dst += img.getWidth() * yOffset;
    dst += xOffset;
    for(int32_t y = 0; y < scaledHeight; ++y)
    {
      std::copy_n(reinterpret_cast<gl::SRGBA8*>(srcLineRaw), // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
                  scaledWidth,
                  dst);
      dst += img.getWidth();
      srcLineRaw += dstVideoLinesize[0];
    }
  }
};

void play(const std::filesystem::path& filename,
          SoLoud::Soloud& soLoud,
          const std::shared_ptr<gl::Image<gl::SRGBA8>>& img,
          const std::function<bool()>& onFrame)
{
  if(!is_regular_file(filename))
    BOOST_THROW_EXCEPTION(std::runtime_error("Video file not found"));

  auto decoderPtr = std::make_unique<AVDecoder>(filename.string());
  const auto decoder = decoderPtr.get();
  Expects(decoder->filterGraph.graph->sink_links_count == 1);
  Scaler sws{decoder->filterGraph.graph->sink_links[0]};

  const auto handle = soLoud.play(*decoderPtr);

  const auto streamFinisher = gsl::finally([&handle, &soLoud]() { soLoud.stop(handle); });

  while(!decoder->stopped)
  {
    if(const auto f = decoder->takeFrame())
    {
      sws.resize(img->getWidth(), img->getHeight());
      sws.scale(*f, *img);
    }

    decoder->stopped |= !onFrame();
  }
}
} // namespace video
