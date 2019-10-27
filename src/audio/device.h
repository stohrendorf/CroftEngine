#pragma once

#include "filterhandle.h"
#include "gsl-lite.hpp"
#include "sourcehandle.h"
#include "stream.h"

#include <AL/alc.h>
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

  const std::shared_ptr<FilterHandle>& getUnderwaterFilter() const
  {
    return m_underwaterFilter;
  }

  void removeStream(const std::shared_ptr<Stream>& stream)
  {
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

  // ReSharper disable once CppMemberFunctionMayBeConst
  // ReSharper disable once CppMemberFunctionMayBeStatic
  void setListenerTransform(const glm::vec3& pos, const glm::vec3& front, const glm::vec3& up)
  {
    AL_ASSERT(alListener3f(AL_POSITION, pos.x, pos.y, pos.z));

    const ALfloat o[6] = {front.x, front.y, front.z, up.x, up.y, up.z};
    AL_ASSERT(alListenerfv(AL_ORIENTATION, o));
  }

  void applyDirectFilterToAllSources(const std::shared_ptr<FilterHandle>& filter)
  {
    for(const auto& src : m_sources)
      src->setDirectFilter(filter);
  }

  gsl::not_null<std::shared_ptr<Stream>>
    createStream(std::unique_ptr<AbstractStreamSource>&& src, const size_t bufferSize, const size_t bufferCount)
  {
    const auto r = std::make_shared<Stream>(*this, std::move(src), bufferSize, bufferCount);
    m_streams.emplace(r);
    return r;
  }

  gsl::not_null<std::shared_ptr<SourceHandle>> createSource()
  {
    const auto r = std::make_shared<SourceHandle>();
    m_sources.emplace(r);
    return r;
  }

private:
  ALCdevice* m_device = nullptr;
  ALCcontext* m_context = nullptr;
  std::shared_ptr<FilterHandle> m_underwaterFilter = nullptr;
  std::set<std::shared_ptr<SourceHandle>> m_sources;
  std::set<std::shared_ptr<Stream>> m_streams;
  std::thread m_streamUpdater;
  bool m_shutdown = false;

  void updateStreams()
  {
    for(const auto& stream : m_streams)
      stream->update();
  }
};
} // namespace audio
