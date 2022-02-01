#pragma once

#include "voice.h"

#include <AL/al.h>
#include <chrono>
#include <functional>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <utility>

namespace audio
{
class FadeVolumeCallback final
{
public:
  using FinalCallback = std::function<void()>;

  explicit FadeVolumeCallback(ALfloat to,
                              const std::chrono::high_resolution_clock::duration& duration,
                              const gslu::nn_shared<Voice>& voice,
                              FinalCallback finalCallback)
      : m_from{voice->getLocalGain()}
      , m_to{to}
      , m_duration{duration}
      , m_voice{voice}
      , m_finalCallback{std::move(finalCallback)}
  {
  }

  bool operator()(const std::chrono::high_resolution_clock::duration& t)
  {
    auto voice = m_voice.lock();
    if(voice == nullptr)
    {
      if(static_cast<bool>(m_finalCallback))
        m_finalCallback();
      return false;
    }

    if(t > m_duration)
    {
      voice->setLocalGainLogarithmic(m_to);
      if(static_cast<bool>(m_finalCallback))
        m_finalCallback();
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
  FinalCallback m_finalCallback;
};
} // namespace audio