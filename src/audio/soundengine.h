#pragma once

#include <AL/al.h>
#include <cstddef>
#include <glm/vec3.hpp>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace audio
{
class Device;
class Voice;
class BufferVoice;
class BufferHandle;
class Listener;
class Emitter;

class SoundEngine final
{
  friend class Emitter;
  friend class Listener;

public:
  explicit SoundEngine();
  ~SoundEngine();

  gsl::not_null<std::shared_ptr<BufferVoice>> playBuffer(const gsl::not_null<std::shared_ptr<BufferHandle>>& buffer,
                                                         size_t bufferId,
                                                         ALfloat pitch,
                                                         ALfloat volume,
                                                         const glm::vec3& pos);

  gsl::not_null<std::shared_ptr<BufferVoice>> playBuffer(const gsl::not_null<std::shared_ptr<BufferHandle>>& buffer,
                                                         size_t bufferId,
                                                         ALfloat pitch,
                                                         ALfloat volume,
                                                         Emitter* emitter = nullptr);

  bool stopBuffer(size_t bufferId, Emitter* emitter);

  std::vector<gsl::not_null<std::shared_ptr<Voice>>> getVoicesForBuffer(Emitter* emitter, size_t buffer) const;

  [[nodiscard]] const auto& getDevice() const noexcept
  {
    return *m_device;
  }

  [[nodiscard]] auto& getDevice() noexcept
  {
    return *m_device;
  }

  void setListener(const Listener* listener)
  {
    m_listener = listener;
  }

  void setListenerGain(float gain);

  void update();

  void dropEmitter(Emitter* emitter);

  void reset();

private:
  const gsl::not_null<std::unique_ptr<Device>> m_device;
  std::unordered_map<Emitter*, std::unordered_map<size_t, std::vector<std::weak_ptr<Voice>>>> m_voices;
  const Listener* m_listener = nullptr;

  std::unordered_set<Emitter*> m_emitters;
  std::unordered_set<Listener*> m_listeners;
};
} // namespace audio
