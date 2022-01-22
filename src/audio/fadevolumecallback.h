#pragma once

#include "voice.h"

#include <AL/al.h>
#include <chrono>
#include <gsl/gsl-lite.hpp>

namespace audio
{
class FadeVolumeCallback final
{
public:
  explicit FadeVolumeCallback(ALfloat to,
                              const std::chrono::high_resolution_clock::duration& duration,
                              const gsl::not_null<std::shared_ptr<Voice>>& voice)
      : m_from{voice->getLocalGain()}
      , m_to{to}
      , m_duration{duration}
      , m_voice{voice}
  {
  }

  bool operator()(const std::chrono::high_resolution_clock::duration& t)
  {
    auto voice = m_voice.lock();
    if(voice == nullptr)
    {
      return false;
    }

    if(t > m_duration)
    {
      voice->setLocalGainLogarithmic(m_to);
      return false;
    }

    const auto value = m_from + (m_to - m_from) * t / m_duration;
    voice->setLocalGainLogarithmic(value);
    return true;
  }

private:
  const float m_from;
  const float m_to;
  const std::chrono::high_resolution_clock::duration m_duration;
  const std::weak_ptr<Voice> m_voice;
};
} // namespace audio