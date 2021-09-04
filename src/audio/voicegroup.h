#pragma once

#include "voice.h"

namespace audio
{
class VoiceGroup final
{
private:
  float m_gain;
  std::vector<gsl::not_null<std::shared_ptr<Voice>>> m_voices{};

public:
  explicit VoiceGroup(float gain = 1.0f)
      : m_gain{gain}
  {
  }

  void add(gsl::not_null<std::shared_ptr<Voice>> voice)
  {
    voice->setGroupGain(m_gain);
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
                 [](const auto& voice) { return !voice->isStopped(); });
    m_voices = std::move(cleaned);
  }

  [[nodiscard]] float getGain() const
  {
    return m_gain;
  }

  void setGain(float gain)
  {
    m_gain = gain;
    for(const auto& voice : m_voices)
      voice->setGroupGain(gain);
  }
};
} // namespace audio
