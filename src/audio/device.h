#pragma once

#include <AL/alc.h>
#include <array>
#include <chrono>
#include <glm/vec3.hpp>
#include <gsl/gsl-lite.hpp>
#include <mutex>
#include <set>
#include <thread>
#include <unordered_set>
#include <vector>

namespace audio
{
class Voice;
class StreamVoice;
class FilterHandle;
class AbstractStreamSource;

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

  void removeStream(const std::shared_ptr<StreamVoice>& stream);

  void setListenerTransform(const glm::vec3& pos, const glm::vec3& front, const glm::vec3& up);

  void setListenerGain(float gain);

  void setFilter(const std::shared_ptr<FilterHandle>& filter)
  {
    m_filter = filter;
  }

  [[nodiscard]] gsl::not_null<std::shared_ptr<StreamVoice>>
    createStream(std::unique_ptr<AbstractStreamSource>&& src,
                 size_t bufferSize,
                 size_t bufferCount,
                 const std::chrono::milliseconds& initialPosition);

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

  void updateStreams();
};
} // namespace audio
