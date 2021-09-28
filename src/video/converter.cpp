#include "converter.h"

#include "avframeptr.h"

#include <algorithm>
#include <boost/throw_exception.hpp>
#include <cstddef>
#include <gl/pixel.h>
#include <gl/sampler.h>
#include <gl/texture2d.h>
#include <gl/texturehandle.h>
#include <glm/vec2.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <stdexcept>
#include <utility>
#include <vector>

extern "C"
{
#include <libavfilter/avfilter.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

namespace video
{
constexpr auto OutputPixFmt = AV_PIX_FMT_RGBA;

Converter::Converter(AVFilterLink* filter)
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
    , textureHandle{std::make_shared<gl::TextureHandle<gl::Texture2D<gl::SRGBA8>>>(
        gslu::make_nn_shared<gl::Texture2D<gl::SRGBA8>>(glm::ivec2{filter->w, filter->h}, "video"),
        gslu::make_nn_unique<gl::Sampler>("video") | set(gl::api::TextureMagFilter::Linear))}
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

void Converter::update(const AVFramePtr& videoFrame)
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

  textureHandle->getTexture()->assign(dstData);
}
} // namespace video
