#pragma once

#include <AL/al.h>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>
#include <vector>

namespace audio
{
class Voice;

class VoiceGroup final
{
private:
  ALfloat m_gain;
  std::vector<std::weak_ptr<Voice>> m_voices{};

public:
  explicit VoiceGroup(ALfloat gain = 1.0f)
      : m_gain{gain}
  {
  }

  void add(gslu::nn_shared<Voice> voice);

  void remove(const gslu::nn_shared<Voice>& voice);

  void cleanup();

  [[nodiscard]] ALfloat getGain() const
  {
    return m_gain;
  }

  void setGain(ALfloat gain);
};
} // namespace audio
