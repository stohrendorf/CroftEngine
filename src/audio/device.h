#pragma once

#include "filterhandle.h"
#include "sourcehandle.h"
#include "streamvoice.h"

#include <AL/alc.h>
#include <gsl/gsl-lite.hpp>
#include <mutex>
#include <set>
#include <thread>

namespace audio
{
class Device final
{
public:
  explicit Device();

  explicit Device(const Device&) = delete;
  explicit Device(Device&&) = delete;
  Device& operator=(const Device&) = delete;
  Device& operator=(Device&&) = delete;

  ~Device();

  void update()
  {
    removeStoppedSources();
  }

  [[nodiscard]] const auto& getUnderwaterFilter() const
  {
    return m_underwaterFilter;
  }

  void removeStream(const std::shared_ptr<StreamVoice>& stream)
  {
    stream->setLooping(false);
    stream->stop();
    std::lock_guard lock{m_streamsLock};
    m_streams.erase(stream);
  }

  void removeStoppedSources()
  {
    std::set<std::shared_ptr<SourceHandle>> cleaned;
    std::copy_if(m_sources.begin(),
                 m_sources.end(),
                 std::inserter(cleaned, cleaned.end()),
                 [](const std::shared_ptr<SourceHandle>& src) { return !src->isStopped(); });

    m_sources = std::move(cleaned);
  }

  // NOLINTNEXTLINE(readability-make-member-function-const, readability-convert-member-functions-to-static)
  void setListenerTransform(const glm::vec3& pos, const glm::vec3& front, const glm::vec3& up)
  {
    AL_ASSERT(alListener3f(AL_POSITION, pos.x, pos.y, pos.z));

    const std::array<ALfloat, 6> o{front.x, front.y, front.z, up.x, up.y, up.z};
    AL_ASSERT(alListenerfv(AL_ORIENTATION, o.data()));
  }

  // NOLINTNEXTLINE(readability-make-member-function-const, readability-convert-member-functions-to-static)
  void setListenerGain(float gain)
  {
    Expects(gain >= 0);
    AL_ASSERT(alListenerf(AL_GAIN, gain));
  }

  void applyDirectFilterToAllSources(const std::shared_ptr<FilterHandle>& filter)
  {
    for(const auto& src : m_sources)
      src->setDirectFilter(filter);
  }

  [[nodiscard]] gsl::not_null<std::shared_ptr<StreamVoice>>
    createStream(std::unique_ptr<AbstractStreamSource>&& src, const size_t bufferSize, const size_t bufferCount)
  {
    const auto r = std::make_shared<StreamVoice>(*this, std::move(src), bufferSize, bufferCount);

    std::lock_guard lock{m_streamsLock};
    m_streams.emplace(r);
    return r;
  }

  [[nodiscard]] gsl::not_null<std::shared_ptr<SourceHandle>> createSourceHandle()
  {
    const auto r = std::make_shared<SourceHandle>();
    m_sources.emplace(r);
    return r;
  }

  [[nodiscard]] gsl::not_null<std::shared_ptr<SourceHandle>> createStreamingSourceHandle()
  {
    return std::make_shared<StreamingSourceHandle>();
  }

  void reset();

private:
  ALCdevice* m_device = nullptr;
  ALCcontext* m_context = nullptr;
  std::shared_ptr<FilterHandle> m_underwaterFilter = nullptr;
  std::set<std::shared_ptr<SourceHandle>> m_sources;
  std::set<std::shared_ptr<StreamVoice>> m_streams;
  std::thread m_streamUpdater;
  std::recursive_mutex m_streamsLock;
  bool m_shutdown = false;

  void updateStreams()
  {
    std::lock_guard lock{m_streamsLock};
    for(const auto& stream : m_streams)
      stream->update();
  }
};
} // namespace audio
