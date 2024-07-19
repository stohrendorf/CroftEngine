#include "converter.h"

#include "ffmpeg/avframeptr.h"

#include <algorithm>
#include <boost/throw_exception.hpp>
#include <cstddef>
#include <cstdint>
#include <gl/constants.h>
#include <gl/pixel.h>
#include <gl/sampler.h>
#include <gl/texture2d.h>
#include <gl/texturehandle.h>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <stdexcept>
#include <vector>

extern "C"
{
#include <libavfilter/avfilter.h>
#include <libavutil/imgutils.h>
#include <libavutil/mem.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
}

namespace video
{
constexpr auto OutputPixFmt = AV_PIX_FMT_RGB24;

Converter::Converter(const glm::ivec2& size, AVPixelFormat srcFormat)
    : size{size}
    , context{sws_getContext(size.x,
                             size.y,
                             srcFormat,
                             size.x,
                             size.y,
                             OutputPixFmt,
                             SWS_POINT | SWS_FULL_CHR_H_INT | SWS_ACCURATE_RND,
                             nullptr,
                             nullptr,
                             nullptr)}
    , textureHandle{std::make_shared<gl::TextureHandle<gl::Texture2D<gl::SRGB8>>>(
        gsl::make_shared<gl::Texture2D<gl::SRGB8>>(glm::ivec2{size.x, size.y}, "video"),
        gsl::make_unique<gl::Sampler>("video" + gl::SamplerSuffix) | set(gl::api::TextureMagFilter::Linear))}
{
  if(context == nullptr)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create SWS context"));
  }
}

Converter::~Converter()
{
  sws_freeContext(context);
  av_freep(dstVideoData.data());
}

void Converter::update(const ffmpeg::AVFramePtr& videoFrame)
{
  gsl_Expects(videoFrame.frame->width == size.x && videoFrame.frame->height == size.y);
  gsl_Expects((textureHandle->getTexture()->size() == glm::ivec2{size.x, size.y}));
  av_freep(dstVideoData.data());
  if(av_image_alloc(dstVideoData.data(), dstVideoLinesize.data(), size.x, size.y, OutputPixFmt, 1) < 0)
  {
    BOOST_THROW_EXCEPTION(std::runtime_error("Could not allocate raw video buffer"));
  }

  gsl_Assert(sws_scale(context,
                       static_cast<const uint8_t* const*>(videoFrame.frame->data),
                       videoFrame.frame->linesize,
                       0,
                       videoFrame.frame->height,
                       dstVideoData.data(),
                       dstVideoLinesize.data())
             == size.y);

  std::vector<gl::SRGB8> dstData;
  dstData.resize(gsl::narrow_cast<size_t>(size.x * size.y), {0, 0, 0});

  // TODO this should get fixed
  gsl_Assert(dstVideoLinesize[0] == size.x * 3);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  std::copy_n(reinterpret_cast<gl::SRGB8*>(dstVideoData[0]), size.x * size.y, dstData.data());

  textureHandle->getTexture()->assign(dstData);
}
} // namespace video
