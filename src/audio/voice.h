#pragma once

#include <glm/glm.hpp>
#include <gsl-lite.hpp>
#include <soloud.h>

namespace audio
{
class Voice
{
private:
  const gsl::not_null<std::shared_ptr<SoLoud::Soloud>> m_soLoud;
  const gsl::not_null<std::shared_ptr<SoLoud::AudioSource>> m_source;
  const SoLoud::handle m_voiceHandle;

public:
  explicit Voice(gsl::not_null<std::shared_ptr<SoLoud::Soloud>> soLoud,
                 gsl::not_null<std::shared_ptr<SoLoud::AudioSource>> source,
                 SoLoud::handle voiceHandle)
      : m_soLoud{std::move(soLoud)}
      , m_source{source}
      , m_voiceHandle{voiceHandle}
  {
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
    m_soLoud->setVolume(m_voiceHandle, volume);
  }

  void setRelativePlaySpeed(float factor)
  {
    m_soLoud->setRelativePlaySpeed(m_voiceHandle, factor);
  }

  void setPosition(const glm::vec3& pos)
  {
    m_soLoud->set3dSourcePosition(m_voiceHandle, pos.x, pos.y, pos.z);
  }

  void init3dParams(float maxDistance)
  {
    m_soLoud->set3dSourceAttenuation(m_voiceHandle, SoLoud::AudioSource::LINEAR_DISTANCE, 1);
    m_soLoud->set3dSourceMinMaxDistance(m_voiceHandle, 0, maxDistance);
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

  bool isValid() const
  {
    return m_soLoud->isValidVoiceHandle(m_voiceHandle);
  }

  ~Voice()
  {
    stop();
  }
};
} // namespace audio
