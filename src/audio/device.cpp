#include "device.h"

#include "filterhandle.h"
#include "loadefx.h"
#include "sourcehandle.h"
#include "streamvoice.h"
#include "utils.h"
#include "voice.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <algorithm>
#include <array>
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <stdexcept>
#include <utility>

#ifdef WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif

namespace
{
// simple utility function to ensure the first parameter is actually a symbol known to the compiler
template<typename T>
constexpr auto identityCompileCheck(T, const char* s)
{
  return s;
}
} // namespace

#define CE_STRINGIFY(x) identityCompileCheck(x, #x)

namespace audio
{
namespace
{
const std::array<ALCint, 5> deviceQueryParamList{// reserve additional 2 sources for audio tracks
                                                 ALC_STEREO_SOURCES,
                                                 Device::SourceHandleSlots + 2,
                                                 ALC_SYNC,
                                                 ALC_FALSE,
                                                 ALC_INVALID};

void logDeviceInfo(const gsl::not_null<ALCdevice*>& device)
{
  BOOST_LOG_TRIVIAL(info) << "OpenAL device: " << alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
  BOOST_LOG_TRIVIAL(info) << "OpenAL version: " << AL_ASSERT_FN(alGetString(AL_VERSION));
  BOOST_LOG_TRIVIAL(info) << "OpenAL vendor: " << AL_ASSERT_FN(alGetString(AL_VENDOR));
  BOOST_LOG_TRIVIAL(info) << "OpenAL renderer: " << AL_ASSERT_FN(alGetString(AL_RENDERER));

  {
    ALCint frq;
    ALC_ASSERT(device, alcGetIntegerv(device, ALC_FREQUENCY, 1, &frq));
    ALCint stereoSources;
    ALC_ASSERT(device, alcGetIntegerv(device, ALC_STEREO_SOURCES, 1, &stereoSources));
    BOOST_LOG_TRIVIAL(info) << "OpenAL: Frequency " << frq << ", stereo sources " << stereoSources;
  }

  if(alcIsExtensionPresent(device, CE_STRINGIFY(ALC_SOFT_HRTF)) != ALC_TRUE)
  {
    BOOST_LOG_TRIVIAL(info) << "ALC_SOFT_HRTF extension not present";
  }
  else
  {
    ALCint value = 0;
    ALC_ASSERT(device, alcGetIntegerv(device, ALC_HRTF_STATUS_SOFT, 1, &value));
    switch(value)
    {
    case ALC_HRTF_DISABLED_SOFT:
      BOOST_LOG_TRIVIAL(info) << "HRTF is disabled";
      break;
    case ALC_HRTF_ENABLED_SOFT:
      BOOST_LOG_TRIVIAL(info) << "HRTF is enabled";
      break;
    case ALC_HRTF_DENIED_SOFT:
      BOOST_LOG_TRIVIAL(info) << "HRTF is not allowed on this device";
      break;
    case ALC_HRTF_REQUIRED_SOFT:
      BOOST_LOG_TRIVIAL(info) << "HRTF is required on this device";
      break;
    case ALC_HRTF_HEADPHONES_DETECTED_SOFT:
      BOOST_LOG_TRIVIAL(info) << "HRTF is enabled because headphones were detected";
      break;
    case ALC_HRTF_UNSUPPORTED_FORMAT_SOFT:
      BOOST_LOG_TRIVIAL(info) << "HRTF is disabled because of an incompatible format";
      break;
    default:
      BOOST_LOG_TRIVIAL(warning) << "Unknown HRTF status value #" << value;
      break;
    }
  }
}
} // namespace

Device::~Device()
{
  reset();

  m_shutdown = true;
  m_streamUpdater.join();

  m_underwaterFilter.reset();

  {
    const std::lock_guard lock{m_streamsLock};
    m_streams.clear();
  }

  if(m_context != nullptr)
  {
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(m_context);
  }

  if(m_device != nullptr)
  {
    alcCloseDevice(m_device);
  }
}

Device::Device()
{
  alcGetError(nullptr); // clear any error

  BOOST_LOG_TRIVIAL(info) << "Trying to use default OpenAL device";
  m_device = alcOpenDevice(nullptr);

  if(m_device != nullptr)
  {
    m_context = alcCreateContext(m_device, deviceQueryParamList.data());
  }
  else
  {
    BOOST_LOG_TRIVIAL(error) << "Failed to open default device";
  }

  if(m_context == nullptr)
  {
    BOOST_LOG_TRIVIAL(error) << "Failed to create OpenAL context.";
    if(m_device != nullptr)
      alcCloseDevice(m_device);
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create OpenAL context."));
  }

  alcMakeContextCurrent(m_context);
  AL_ASSERT(alListenerf(AL_METERS_PER_UNIT, 1 / 512.0f));
  AL_ASSERT(alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED));

  loadALExtFunctions(gsl::not_null{m_device});

  logDeviceInfo(gsl::not_null{m_device});

  ALC_ASSERT(m_device, alcGetIntegerv(m_device, ALC_FREQUENCY, 1, &m_frq));

  m_underwaterFilter = std::make_shared<FilterHandle>();

  AL_ASSERT(alFilteri(*m_underwaterFilter, AL_FILTER_TYPE, AL_FILTER_LOWPASS));
  AL_ASSERT(alFilterf(*m_underwaterFilter, AL_LOWPASS_GAIN, 0.7f));   // Low frequencies gain.
  AL_ASSERT(alFilterf(*m_underwaterFilter, AL_LOWPASS_GAINHF, 0.1f)); // High frequencies gain.

  m_streamUpdater = std::thread{[this]()
                                {
                                  while(!this->m_shutdown)
                                  {
                                    this->updateStreams();
                                    std::this_thread::yield();
                                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                                  }
                                }};
#ifdef WIN32
  if(FAILED(SetThreadDescription(m_streamUpdater.native_handle(), L"device stream updater")))
    BOOST_LOG_TRIVIAL(warning) << "Failed to set thread description for audio updater thread";
#endif
}

void Device::reset()
{
  const std::lock_guard lock{m_streamsLock};

  m_updateCallbacks.clear();

  for(const auto& stream : m_streams)
  {
    stream->setLooping(false);
    stream->stop();
  }
  m_streams.clear();

  m_filter.reset();
  for(const auto& voice : m_allVoices)
  {
    if(const auto& src = voice->getSourceHandle())
      src->setDirectFilter(nullptr);
  }
  m_allVoices.clear();
}

void Device::update()
{
  // remove expired streams and voices
  {
    const std::lock_guard lock{m_streamsLock};
    auto streams = std::move(m_streams);
    for(const auto& stream : streams)
    {
      if(!stream->done())
        m_streams.emplace(stream);
    }
  }
  m_allVoices.erase(std::remove_if(m_allVoices.begin(),
                                   m_allVoices.end(),
                                   [](const auto& v)
                                   {
                                     return v->done();
                                   }),
                    m_allVoices.end());

  // order voices by non-positional, then by distance
  glm::vec3 listenerPos;
  AL_ASSERT(alGetListener3f(AL_POSITION, &listenerPos.x, &listenerPos.y, &listenerPos.z));
  std::stable_sort(m_allVoices.begin(),
                   m_allVoices.end(),
                   [&listenerPos](const auto& a, const auto& b)
                   {
                     // non-positional voices have the highest priority
                     const auto aDist = a->isPositional() ? glm::distance(listenerPos, *a->getPosition()) : 0.0f;
                     const auto bDist = b->isPositional() ? glm::distance(listenerPos, *b->getPosition()) : 0.0f;
                     return aDist < bDist;
                   });

  size_t vi = 0;
  for(const auto& voice : m_allVoices)
  {
    if(voice->isPaused())
    {
      voice->associate(nullptr);
      continue;
    }

    ++vi;
    if(vi < SourceHandleSlots)
    {
      if(!voice->hasSourceHandle())
        voice->associate(std::make_unique<SourceHandle>(voice->isPositional()));
      voice->getSourceHandle()->setDirectFilter(m_filter);
    }
    else
    {
      voice->associate(nullptr);
    }
  }
}

gslu::nn_shared<StreamVoice> Device::createStream(std::unique_ptr<AbstractStreamSource>&& src,
                                                  const size_t bufferSize,
                                                  const size_t bufferCount,
                                                  const std::chrono::milliseconds& initialPosition)
{
  auto stream = gsl::make_shared<StreamVoice>(
    std::make_unique<StreamingSourceHandle>(), std::move(src), bufferSize, bufferCount, initialPosition);

  const std::lock_guard lock{m_streamsLock};
  m_streams.emplace(stream);
  return stream;
}

void Device::updateStreams()
{
  const std::lock_guard lock{m_streamsLock};
  for(const auto& stream : m_streams)
    stream->update();
  auto tmp = std::move(m_updateCallbacks);
  for(auto& [fn, t] : tmp)
  {
    if(fn(std::chrono::high_resolution_clock::now() - t))
    {
      m_updateCallbacks.emplace_back(fn, t);
    }
  }
}

void Device::removeStream(const gslu::nn_shared<StreamVoice>& stream)
{
  stream->setLooping(false);
  stream->stop();
  const std::lock_guard lock{m_streamsLock};
  m_streams.erase(stream);
}

// NOLINTNEXTLINE(readability-make-member-function-const, readability-convert-member-functions-to-static)
void Device::setListenerTransform(const glm::vec3& pos, const glm::vec3& front, const glm::vec3& up)
{
  AL_ASSERT(alListener3f(AL_POSITION, pos.x, pos.y, pos.z));

  const std::array<ALfloat, 6> o{front.x, front.y, front.z, up.x, up.y, up.z};
  AL_ASSERT(alListenerfv(AL_ORIENTATION, o.data()));
}

// NOLINTNEXTLINE(readability-make-member-function-const, readability-convert-member-functions-to-static)
void Device::setListenerGain(float gain)
{
  gsl_Expects(gain >= 0);
  AL_ASSERT(alListenerf(AL_GAIN, gain));
}

void Device::registerUpdateCallback(const std::function<UpdateCallback>& fn)
{
  const std::lock_guard lock{m_streamsLock};
  m_updateCallbacks.emplace_back(fn, std::chrono::high_resolution_clock::now());
}
} // namespace audio
