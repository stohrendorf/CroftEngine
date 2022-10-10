#include "voicegroup.h"

#include "voice.h"

#include <AL/al.h>
#include <algorithm>
#include <iterator>
#include <utility>

namespace audio
{
void VoiceGroup::add(gslu::nn_shared<Voice> voice)
{
  voice->setGroupGainLogarithmic(m_gain);
  if(std::find_if(m_voices.begin(),
                  m_voices.end(),
                  [&voice](const auto& weakVoice)
                  {
                    return weakVoice.lock() == voice;
                  })
     == m_voices.end())
  {
    m_voices.emplace_back(voice.get());
  }
}

void VoiceGroup::remove(const gslu::nn_shared<Voice>& voice)
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

void VoiceGroup::cleanup()
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

void VoiceGroup::setGain(ALfloat gain)
{
  m_gain = gain;
  for(const auto& weakVoice : m_voices)
  {
    if(auto v = weakVoice.lock())
      v->setGroupGainLogarithmic(gain);
  }
}
} // namespace audio
