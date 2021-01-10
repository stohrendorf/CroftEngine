#pragma once

#include "filterhandle.h"
#include "sourcehandle.h"
#include "stream.h"

#include <AL/alc.h>
#include <gsl-lite.hpp>
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

  void removeStream(const std::shared_ptr<Stream>& stream)
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

  void applyDirectFilterToAllSources(const std::shared_ptr<FilterHandle>& filter)
  {
    for(const auto& src : m_sources)
      src->setDirectFilter(filter);
  }

  [[nodiscard]] gsl::not_null<std::shared_ptr<Stream>>
    createStream(std::unique_ptr<AbstractStreamSource>&& src, const size_t bufferSize, const size_t bufferCount)
  {
    const auto r = std::make_shared<Stream>(*this, std::move(src), bufferSize, bufferCount);
    r->setGain(m_streamGain);

    std::lock_guard lock{m_streamsLock};
    m_streams.emplace(r);
    return r;
  }

  [[nodiscard]] gsl::not_null<std::shared_ptr<SimpleSourceHandle>>
    createSimpleSource(const std::shared_ptr<BufferHandle>& buffer)
  {
    const auto r = std::make_shared<SimpleSourceHandle>(buffer);
    m_sources.emplace(r);
    return r;
  }

  [[nodiscard]] gsl::not_null<std::shared_ptr<StreamingSourceHandle>> createStreamingSource()
  {
    const auto r = std::make_shared<StreamingSourceHandle>();
    m_sources.emplace(r);
    return r;
  }

  void reset();

  void setStreamGain(ALfloat gain)
  {
    m_streamGain = gain;
    std::lock_guard<std::recursive_mutex> lock{m_streamsLock};
    for(const auto& stream : m_streams)
      stream->setGain(m_streamGain);
  }

  auto getStreamGain() const
  {
    return m_streamGain;
  }

private:
  ALCdevice* m_device = nullptr;
  ALCcontext* m_context = nullptr;
  std::shared_ptr<FilterHandle> m_underwaterFilter = nullptr;
  std::set<std::shared_ptr<SourceHandle>> m_sources;
  std::set<std::shared_ptr<Stream>> m_streams;
  std::thread m_streamUpdater;
  std::recursive_mutex m_streamsLock;
  bool m_shutdown = false;
  ALfloat m_streamGain{0.8f};

  void updateStreams()
  {
    std::lock_guard lock{m_streamsLock};
    for(const auto& stream : m_streams)
      stream->update();
  }
};
} // namespace audio
