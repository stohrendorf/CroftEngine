#pragma once

#include "filterhandle.h"
#include "sourcehandle.h"
#include "streamvoice.h"

#include <AL/alc.h>
#include <array>
#include <chrono>
#include <gsl/gsl-lite.hpp>
#include <mutex>
#include <set>
#include <thread>
#include <unordered_set>

namespace audio
{
class Device final
{
public:
  static constexpr size_t SourceHandleSlots = 32;

  explicit Device();

  explicit Device(const Device&) = delete;
  explicit Device(Device&&) = delete;
  Device& operator=(const Device&) = delete;
  Device& operator=(Device&&) = delete;

  ~Device();

  void update();

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

  void setFilter(const std::shared_ptr<FilterHandle>& filter)
  {
    m_filter = filter;
  }

  [[nodiscard]] gsl::not_null<std::shared_ptr<StreamVoice>>
    createStream(std::unique_ptr<AbstractStreamSource>&& src,
                 const size_t bufferSize,
                 const size_t bufferCount,
                 const std::chrono::milliseconds& initialPosition)
  {
    const auto r = std::make_shared<StreamVoice>(
      std::make_unique<StreamingSourceHandle>(), std::move(src), bufferSize, bufferCount, initialPosition);

    std::lock_guard lock{m_streamsLock};
    m_streams.emplace(r);
    return r;
  }

  void reset();

  void registerVoice(const std::shared_ptr<Voice>& voice)
  {
    m_allVoices.emplace_back(voice);
  }

private:
  ALCdevice* m_device = nullptr;
  ALCcontext* m_context = nullptr;
  std::shared_ptr<FilterHandle> m_underwaterFilter = nullptr;
  std::vector<std::shared_ptr<Voice>> m_allVoices;
  std::set<std::shared_ptr<StreamVoice>> m_streams;
  std::thread m_streamUpdater;
  std::recursive_mutex m_streamsLock;
  bool m_shutdown = false;
  std::shared_ptr<FilterHandle> m_filter{nullptr};
  std::chrono::system_clock::time_point m_lastLogTime = std::chrono::system_clock::now();

  void updateStreams()
  {
    std::lock_guard lock{m_streamsLock};
    for(const auto& stream : m_streams)
      stream->update();
  }
};
} // namespace audio
