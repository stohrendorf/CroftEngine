#pragma once

#include "voice.h"

namespace audio
{
class VoiceGroup final
{
private:
  float m_gain;
  std::vector<std::weak_ptr<Voice>> m_voices{};

public:
  explicit VoiceGroup(float gain = 1.0f)
      : m_gain{gain}
  {
  }

  void add(gsl::not_null<std::shared_ptr<Voice>> voice)
  {
    voice->setGroupGain(m_gain);
    if(std::find_if(
         m_voices.begin(), m_voices.end(), [&voice](const auto& weakVoice) { return weakVoice.lock() == voice; })
       == m_voices.end())
      m_voices.emplace_back(voice.get());
  }

  void remove(const gsl::not_null<std::shared_ptr<Voice>>& voice)
  {
    decltype(m_voices) cleaned;
    std::copy_if(m_voices.begin(),
                 m_voices.end(),
                 std::back_inserter(cleaned),
                 [&voice](const auto& weakVoice)
                 {
                   if(auto v = weakVoice.lock())
                     return v == voice;
                   else
                     return false;
                 });
    m_voices = std::move(cleaned);
  }

  void cleanup()
  {
    decltype(m_voices) cleaned;
    std::copy_if(m_voices.begin(),
                 m_voices.end(),
                 std::back_inserter(cleaned),
                 [](const auto& weakVoice)
                 {
                   if(auto v = weakVoice.lock())
                     return !v->isStopped();
                   else
                     return false;
                 });
    m_voices = std::move(cleaned);
  }

  [[nodiscard]] float getGain() const
  {
    return m_gain;
  }

  void setGain(float gain)
  {
    m_gain = gain;
    for(const auto& weakVoice : m_voices)
    {
      if(auto v = weakVoice.lock())
        v->setGroupGain(gain);
    }
  }
};
} // namespace audio
