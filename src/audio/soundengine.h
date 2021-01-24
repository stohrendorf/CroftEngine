#pragma once

#include "util.h"
#include "voice.h"

#include <glm/glm.hpp>
#include <gsl-lite.hpp>
#include <soloud_biquadresonantfilter.h>
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
  virtual ~Emitter();

  Emitter(const Emitter& rhs)
      : Emitter{rhs.m_engine}
  {
  }

  Emitter() = delete;

  Emitter& operator=(const Emitter& rhs);
  Emitter(Emitter&& rhs);
  Emitter& operator=(Emitter&& rhs);
  [[nodiscard]] virtual glm::vec3 getPosition() const = 0;

private:
  mutable SoundEngine* m_engine = nullptr;
};

class Listener
{
  friend class SoundEngine;

public:
  explicit Listener(const gsl::not_null<SoundEngine*>& engine);
  virtual ~Listener();

  Listener(const Listener& rhs)
      : Listener{rhs.m_engine}
  {
  }

  Listener() = delete;

  Listener& operator=(const Listener& rhs);
  Listener(Listener&& rhs);
  Listener& operator=(Listener&&);

  [[nodiscard]] virtual glm::vec3 getPosition() const = 0;
  [[nodiscard]] virtual glm::vec3 getFrontVector() const = 0;
  [[nodiscard]] virtual glm::vec3 getUpVector() const = 0;

private:
  mutable SoundEngine* m_engine = nullptr;
};

class SoundEngine final
{
  friend class Emitter;
  friend class Listener;

public:
  SoundEngine();

  ~SoundEngine();

  gsl::not_null<std::shared_ptr<Voice>>
    play(const std::shared_ptr<SoLoud::AudioSource>& audioSource, float pitch, float volume, const glm::vec3& pos);

  gsl::not_null<std::shared_ptr<Voice>> play(const std::shared_ptr<SoLoud::AudioSource>& audioSource,
                                             float pitch,
                                             float volume,
                                             Emitter* emitter = nullptr);

  bool stop(const std::shared_ptr<SoLoud::AudioSource>& audioSource, Emitter* emitter);

  std::vector<gsl::not_null<std::shared_ptr<Voice>>>
    getVoicesForAudioSource(Emitter* emitter, const std::shared_ptr<SoLoud::AudioSource>& audioSource) const;

  [[nodiscard]] const auto& getSoLoud() const noexcept
  {
    return *m_soLoud;
  }

  [[nodiscard]] auto& getSoLoud() noexcept
  {
    return *m_soLoud;
  }

  void setListener(const Listener* listener)
  {
    m_listener = listener;
  }

  void update();

  void dropEmitter(Emitter* emitter);

  void reset();

  SoLoud::Filter& getUnderwaterFilter()
  {
    return m_underwaterFilter;
  }

  gsl::not_null<std::shared_ptr<Voice>> playBackground(const std::shared_ptr<SoLoud::AudioSource>& audioSource,
                                                       float volume)
  {
    return std::make_shared<audio::Voice>(m_soLoud, audioSource, m_soLoud->playBackground(*audioSource, volume));
  }

private:
  gsl::not_null<std::shared_ptr<SoLoud::Soloud>> m_soLoud;
  std::unordered_map<
    Emitter*,
    std::unordered_map<std::shared_ptr<SoLoud::AudioSource>, std::vector<gsl::not_null<std::shared_ptr<audio::Voice>>>>>
    m_voices;
  const Listener* m_listener = nullptr;

  std::unordered_set<Emitter*> m_emitters;
  std::unordered_set<Listener*> m_listeners;

  SoLoud::BiquadResonantFilter m_underwaterFilter{};
};
} // namespace audio
