#pragma once

#include <AL/alc.h>
#include <chrono>
#include <cstddef>
#include <functional>
#include <glm/vec3.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
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
  using UpdateCallback = bool(const std::chrono::high_resolution_clock::duration& dt);

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

  void removeStream(const gslu::nn_shared<StreamVoice>& stream);
  void removeStream(const std::weak_ptr<StreamVoice>& stream)
  {
    if(const auto locked = stream.lock())
      removeStream(gsl::not_null{locked});
  }

  void setListenerTransform(const glm::vec3& pos, const glm::vec3& front, const glm::vec3& up);

  void setListenerGain(float gain);

  void setFilter(const std::shared_ptr<FilterHandle>& filter)
  {
    m_filter = filter;
  }

  [[nodiscard]] gslu::nn_shared<StreamVoice> createStream(std::unique_ptr<AbstractStreamSource>&& src,
                                                          size_t bufferSize,
                                                          size_t bufferCount,
                                                          const std::chrono::milliseconds& initialPosition);

  void reset();

  void registerVoice(const std::shared_ptr<Voice>& voice)
  {
    m_allVoices.emplace_back(voice);
  }

  void registerUpdateCallback(const std::function<UpdateCallback>& fn);

  [[nodiscard]] ALCint getSampleRate() const
  {
    return m_frq;
  }

private:
  ALCdevice* m_device = nullptr;
  ALCcontext* m_context = nullptr;
  std::shared_ptr<FilterHandle> m_underwaterFilter = nullptr;
  std::vector<gslu::nn_shared<Voice>> m_allVoices;
  std::set<gslu::nn_shared<StreamVoice>> m_streams;
  std::thread m_streamUpdater;
  std::vector<std::pair<std::function<UpdateCallback>, std::chrono::high_resolution_clock::time_point>>
    m_updateCallbacks;
  std::recursive_mutex m_streamsLock;
  bool m_shutdown = false;
  std::shared_ptr<FilterHandle> m_filter{nullptr};
  ALCint m_frq = 0;

  void updateStreams();
};
} // namespace audio
