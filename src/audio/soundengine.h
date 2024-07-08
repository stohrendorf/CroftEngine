#pragma once

#include "serialization/serialization_fwd.h"

#include <AL/al.h>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <glm/vec3.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <map>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace engine::world
{
class World;
}

namespace audio
{
class Device;
class Voice;
class BufferVoice;
class BufferHandle;
class Listener;
class Emitter;
class StreamVoice;
class VoiceGroup;

struct SlotStream
{
  std::weak_ptr<StreamVoice> stream;
  std::filesystem::path absolutePath;
};

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

  [[nodiscard]] std::vector<gslu::nn_shared<Voice>> getVoicesForBuffer(Emitter* emitter, size_t buffer) const;

  [[nodiscard]] const auto& getDevice() const noexcept
  {
    return *m_device;
  }

  [[nodiscard]] auto& getDevice() noexcept
  {
    return *m_device;
  }

  void setListener(const Listener* listener) noexcept
  {
    m_listener = listener;
  }

  void setListenerGain(float gain);

  void update();

  void dropEmitter(const Emitter* emitter);

  void reset();

  void
    setSlotStream(size_t slot, const gsl::shared_ptr<StreamVoice>& stream, const std::filesystem::path& absolutePath);
  [[nodiscard]] std::shared_ptr<StreamVoice> tryGetStream(size_t slot);
  void freeSlot(size_t slot) noexcept;
  void serializeStreams(const serialization::Serializer<engine::world::World>& ser);
  void deserializeStreams(const serialization::Deserializer<engine::world::World>& ser,
                          const std::filesystem::path& rootPath,
                          VoiceGroup& streamGroup);
  [[nodiscard]] gslu::nn_shared<StreamVoice> createStream(const std::filesystem::path& path,
                                                          const std::chrono::milliseconds& initialPosition);
  [[nodiscard]] const auto& getSlots() const noexcept
  {
    return m_slots;
  }

private:
  gslu::nn_unique<Device> m_device;
  std::unordered_map<const Emitter*, std::unordered_map<size_t, std::vector<std::weak_ptr<Voice>>>> m_voices;
  const Listener* m_listener = nullptr;

  std::unordered_set<const Emitter*> m_emitters;
  std::unordered_set<Listener*> m_listeners;

  std::map<size_t, SlotStream> m_slots;
};
} // namespace audio
