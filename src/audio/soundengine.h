#pragma once

#include <AL/al.h>
#include <cstddef>
#include <glm/vec3.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
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

  gslu::nn_shared<BufferVoice> playBuffer(
    const gslu::nn_shared<BufferHandle>& buffer, size_t bufferId, ALfloat pitch, ALfloat volume, const glm::vec3& pos);

  gslu::nn_shared<BufferVoice> playBuffer(const gslu::nn_shared<BufferHandle>& buffer,
                                          size_t bufferId,
                                          ALfloat pitch,
                                          ALfloat volume,
                                          Emitter* emitter = nullptr);

  bool stopBuffer(size_t bufferId, const Emitter* emitter);

  std::vector<gslu::nn_shared<Voice>> getVoicesForBuffer(Emitter* emitter, size_t buffer) const;

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

  void dropEmitter(const Emitter* emitter);

  void reset();

private:
  const gslu::nn_unique<Device> m_device;
  std::unordered_map<const Emitter*, std::unordered_map<size_t, std::vector<std::weak_ptr<Voice>>>> m_voices;
  const Listener* m_listener = nullptr;

  std::unordered_set<const Emitter*> m_emitters;
  std::unordered_set<Listener*> m_listeners;
};
} // namespace audio
