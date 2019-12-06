#pragma once

#include "bufferhandle.h"
#include "device.h"
#include "gsl-lite.hpp"
#include "sourcehandle.h"

#include <unordered_map>
#include <unordered_set>

namespace audio
{
class SoundEngine;

class Emitter
{
  friend class SoundEngine;

public:
  explicit Emitter(const gsl::not_null<SoundEngine*>& engine);

  Emitter() = delete;

  Emitter(const Emitter& rhs)
      : Emitter{rhs.m_engine}
  {
  }

  Emitter& operator=(const Emitter& rhs);

  Emitter(Emitter&& rhs) noexcept;

  Emitter& operator=(Emitter&& rhs) noexcept;

  [[nodiscard]] virtual glm::vec3 getPosition() const = 0;

  virtual ~Emitter();

private:
  mutable SoundEngine* m_engine = nullptr;
};

class Listener
{
  friend class SoundEngine;

public:
  explicit Listener(const gsl::not_null<SoundEngine*>& engine);

  Listener() = delete;

  Listener(const Listener& rhs)
      : Listener{rhs.m_engine}
  {
  }

  Listener& operator=(const Listener& rhs);

  Listener(Listener&& rhs) noexcept;

  Listener& operator=(Listener&&) noexcept;

  [[nodiscard]] virtual glm::vec3 getPosition() const = 0;

  [[nodiscard]] virtual glm::vec3 getFrontVector() const = 0;

  [[nodiscard]] virtual glm::vec3 getUpVector() const = 0;

  virtual ~Listener();

private:
  mutable SoundEngine* m_engine = nullptr;
};

class SoundEngine final
{
  friend class Emitter;

  friend class Listener;

public:
  ~SoundEngine();

  void addWav(const gsl::not_null<const uint8_t*>& buffer);

  gsl::not_null<std::shared_ptr<SourceHandle>>
    playBuffer(size_t bufferId, ALfloat pitch, ALfloat volume, const glm::vec3& pos);

  gsl::not_null<std::shared_ptr<SourceHandle>>
    playBuffer(size_t bufferId, ALfloat pitch, ALfloat volume, Emitter* emitter = nullptr);

  bool stopBuffer(size_t bufferId, Emitter* emitter);

  std::vector<gsl::not_null<std::shared_ptr<SourceHandle>>> getSourcesForBuffer(Emitter* emitter, size_t buffer) const;

  [[nodiscard]] const auto& getDevice() const noexcept
  {
    return m_device;
  }

  [[nodiscard]] auto& getDevice() noexcept
  {
    return m_device;
  }

  void setListener(const Listener* listener)
  {
    m_listener = listener;
  }

  void update();

  void dropEmitter(Emitter* emitter);

private:
  Device m_device;
  std::vector<std::shared_ptr<BufferHandle>> m_buffers;
  std::unordered_map<Emitter*, std::unordered_map<size_t, std::vector<std::weak_ptr<SourceHandle>>>> m_sources;
  const Listener* m_listener = nullptr;

  std::unordered_set<Emitter*> m_emitters;
  std::unordered_set<Listener*> m_listeners;
};
}