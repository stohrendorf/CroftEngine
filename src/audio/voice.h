#pragma once

#include "core/magic.h"

#include <algorithm>
#include <chrono>
#include <glm/glm.hpp>
#include <gsl/gsl-lite.hpp>
#include <soloud.h>
#include <utility>
#include <vector>

namespace audio
{
class Voice
{
private:
  const gsl::not_null<std::shared_ptr<SoLoud::Soloud>> m_soLoud;
  const gsl::not_null<std::shared_ptr<SoLoud::AudioSource>> m_source;
  const SoLoud::handle m_voiceHandle;
  float m_baseVolume;

  static constexpr float MaxDistance = 12 * core::SectorSize.get<float>();

public:
  explicit Voice(gsl::not_null<std::shared_ptr<SoLoud::Soloud>> soLoud,
                 gsl::not_null<std::shared_ptr<SoLoud::AudioSource>> source,
                 SoLoud::handle voiceHandle,
                 float baseVolume = 1.0f)
      : m_soLoud{std::move(soLoud)}
      , m_source{std::move(source)}
      , m_voiceHandle{voiceHandle}
      , m_baseVolume{baseVolume}
  {
  }

  virtual ~Voice()
  {
    setProtect(false);
    stop();
  }

  void setLooping(bool looping)
  {
    m_soLoud->setLooping(m_voiceHandle, looping);
  }

  void pause(bool paused = true)
  {
    m_soLoud->setPause(m_voiceHandle, paused);
  }

  void stop()
  {
    m_soLoud->stop(m_voiceHandle);
  }

  void setVolume(float volume)
  {
    m_soLoud->setVolume(m_voiceHandle, volume * m_baseVolume);
  }

  [[nodiscard]] auto getLocalVolume() const
  {
    return m_soLoud->getVolume(m_voiceHandle) / m_baseVolume;
  }

  void setBaseVolume(float baseVolume)
  {
    const auto currentWithoutBase = getLocalVolume();
    m_baseVolume = baseVolume;
    m_soLoud->setVolume(m_voiceHandle, currentWithoutBase * m_baseVolume);
  }

  void fadeVolume(float volume, const std::chrono::milliseconds& time)
  {
    m_soLoud->fadeVolume(m_voiceHandle, volume * m_baseVolume, time.count() / 1000.0);
  }

  void fadeBaseVolume(float baseVolume, const std::chrono::milliseconds& time)
  {
    const auto currentWithoutBase = getLocalVolume();
    m_baseVolume = baseVolume;
    m_soLoud->fadeVolume(m_voiceHandle, currentWithoutBase * m_baseVolume, time.count() / 1000.0);
  }

  void setRelativePlaySpeed(float factor)
  {
    m_soLoud->setRelativePlaySpeed(m_voiceHandle, factor);
  }

  void setPosition(const glm::vec3& pos)
  {
    m_soLoud->set3dSourcePosition(m_voiceHandle, pos.x, pos.y, pos.z);
  }

  void init3dParams()
  {
    m_soLoud->set3dSourceAttenuation(m_voiceHandle, SoLoud::AudioSource::LINEAR_DISTANCE, 1);
    m_soLoud->set3dSourceMinMaxDistance(m_voiceHandle, 0, MaxDistance);
  }

  void play()
  {
    pause(false);
  }

  void restart()
  {
    m_soLoud->seek(m_voiceHandle, 0);
    pause(false);
  }

  [[nodiscard]] bool isValid() const
  {
    return m_soLoud->isValidVoiceHandle(m_voiceHandle);
  }

  void setProtect(bool protect)
  {
    m_soLoud->setProtectVoice(m_voiceHandle, protect);
  }
};

class VoiceCollection final
{
private:
  float m_volume;
  std::vector<gsl::not_null<std::shared_ptr<Voice>>> m_voices{};

public:
  explicit VoiceCollection(float volume = 1.0f)
      : m_volume{volume}
  {
  }

  void add(gsl::not_null<std::shared_ptr<Voice>> voice)
  {
    voice->setBaseVolume(m_volume);
    if(std::find(m_voices.begin(), m_voices.end(), voice) == m_voices.end())
      m_voices.emplace_back(std::move(voice));
  }

  void remove(const gsl::not_null<std::shared_ptr<Voice>>& voice)
  {
    m_voices.erase(std::remove(m_voices.begin(), m_voices.end(), voice), m_voices.end());
  }

  void cleanup()
  {
    decltype(m_voices) cleaned;
    std::copy_if(m_voices.begin(),
                 m_voices.end(),
                 std::back_inserter(cleaned),
                 [](const gsl::not_null<std::shared_ptr<Voice>>& voice) { return voice->isValid(); });
    m_voices = std::move(cleaned);
  }

  [[nodiscard]] float getVolume() const
  {
    return m_volume;
  }

  void setVolume(float volume)
  {
    m_volume = volume;
    for(const auto& voice : m_voices)
      voice->setBaseVolume(volume);
  }

  void fadeVolume(float volume, std::chrono::milliseconds time)
  {
    m_volume = volume;
    for(const auto& voice : m_voices)
      voice->fadeBaseVolume(volume, time);
  }
};
} // namespace audio
