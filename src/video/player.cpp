#include "player.h"

#include "audio/soundengine.h"
#include "audio/stream.h"

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
  int index = -1;
  AVCodecContext* context = nullptr;
  AVStream* stream = nullptr;

  Stream(AVFormatContext* fmtContext, AVMediaType type)
  {
    index = av_find_best_stream(fmtContext, type, -1, -1, nullptr, 0);
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
  AVFrame* frame;

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
                                    AV_CH_LAYOUT_STEREO,
                                    AV_SAMPLE_FMT_S16,
                                    SampleRate,
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
    audioFrameDuration = SampleRate * videoStream->stream->time_base.num / videoStream->stream->time_base.den;

    av_init_packet(&packet);
    packet.data = nullptr;
    packet.size = 0;
    fillQueues();
  }

  bool stopped = false;
  AVPacket packet{};

  void fillQueues()
  {
    {
      std::unique_lock<std::mutex> lock(imgQueueMutex);
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

      std::unique_lock<std::mutex> lock(imgQueueMutex);
      if(audioQueue.size() >= QueueLimit || imgQueue.size() >= QueueLimit)
      {
        break;
      }
    }

    BOOST_LOG_TRIVIAL(trace) << "fillQueues done: " << getAvError(err) << "; audio=" << audioQueue.size()
                             << ", video=" << imgQueue.size();
  }

  std::queue<AVFramePtr> imgQueue;
  std::queue<std::vector<int16_t>> audioQueue;
  mutable std::mutex imgQueueMutex;
  std::condition_variable frameReadyCondition;
  bool frameReady = false;

  auto takeFrame()
  {
    std::unique_lock<std::mutex> lock{imgQueueMutex};
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

  void decodePacket()
  {
    if(packet.stream_index == videoStream->index)
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

          std::unique_lock<std::mutex> lock(imgQueueMutex);
          imgQueue.push(std::move(filteredFrame));
        }
      }
      if(err != AVERROR(EAGAIN))
        BOOST_LOG_TRIVIAL(info) << "Video stream chunk decoded: " << getAvError(err);
    }
    else if(packet.stream_index == audioStream->index)
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

        std::vector<int16_t> audio(outSamples * 2, 0);
        auto* audioData
          = reinterpret_cast<uint8_t*>(audio.data()); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)

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
  }

  static constexpr int SampleRate = 44100;
  size_t audioFrameDuration = 0;
  size_t audioFramePosition = 0;

  size_t readStereo(int16_t* buffer, size_t bufferSize, bool /*looping*/) override
  {
    fillQueues();

    size_t written = 0;
    while(bufferSize != 0 && !audioQueue.empty())
    {
      auto& src = audioQueue.front();
      const auto frames = std::min(bufferSize, src.size() / 2);
      const auto samples = 2 * frames;
      std::copy_n(src.data(), samples, buffer);
      buffer += samples;
      bufferSize -= frames;
      written += frames;

      src.erase(src.begin(), std::next(src.begin(), samples));
      if(src.empty())
      {
        audioQueue.pop();
      }
    }

    std::fill_n(buffer, bufferSize, 0);

    audioFramePosition += written + bufferSize;
    Expects(audioFrameDuration > 0);
    while(audioFramePosition >= audioFrameDuration)
    {
      audioFramePosition -= audioFrameDuration;
      std::unique_lock<std::mutex> lock{imgQueueMutex};
      frameReady = true;
      frameReadyCondition.notify_one();
    }

    return written + bufferSize;
  }

  int getSampleRate() const override
  {
    return SampleRate;
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
          audio::Device& audioDevice,
          const std::shared_ptr<gl::Image<gl::SRGBA8>>& img,
          const std::function<bool()>& onFrame)
{
  if(!is_regular_file(filename))
    BOOST_THROW_EXCEPTION(std::runtime_error("Video file not found"));

  auto decoderPtr = std::make_unique<AVDecoder>(filename.string());
  const auto decoder = decoderPtr.get();
  Expects(decoder->filterGraph.graph->sink_links_count == 1);
  Scaler sws{decoder->filterGraph.graph->sink_links[0]};

  auto stream = audioDevice.createStream(std::move(decoderPtr), decoder->audioFrameDuration, 2);
  stream->setLooping(true);
  stream->play();

  while(!decoder->stopped)
  {
    if(const auto x = stream->getSource().lock())
    {
      if(x->isPaused() || x->isStopped())
        x->play();
    }

    if(const auto f = decoder->takeFrame())
    {
      sws.resize(img->getWidth(), img->getHeight());
      sws.scale(*f, *img);
    }

    decoder->stopped |= !onFrame();
  }

  audioDevice.removeStream(stream);
}
} // namespace video
