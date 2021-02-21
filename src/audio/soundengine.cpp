#include "soundengine.h"

#include <glm/gtx/string_cast.hpp>

namespace audio
{
void SoundEngine::update()
{
  if(m_listener != nullptr)
  {
    const auto pos = m_listener->getPosition();
    m_soLoud->set3dListenerPosition(pos.x, pos.y, pos.z);
    const auto front = m_listener->getFrontVector();
    m_soLoud->set3dListenerAt(front.x, front.y, front.z);
    const auto up = m_listener->getUpVector();
    m_soLoud->set3dListenerUp(up.x, up.y, up.z);
  }
  else
  {
    BOOST_LOG_TRIVIAL(warning) << "No listener set";
  }

  for(auto& [emitter, sourcesAndVoices] : m_voices)
  {
    glm::vec3 pos;
    if(emitter != nullptr)
      pos = emitter->getPosition();

    for(auto& [source, voices] : sourcesAndVoices)
    {
      auto old = std::move(voices);
      std::copy_if(old.begin(), old.end(), std::back_inserter(voices), [](const auto& h) { return h->isValid(); });

      if(emitter == nullptr)
        continue;

      for(const auto& voice : voices)
      {
        voice->setPosition(pos);
      }
    }
  }

  m_soLoud->update3dAudio();
}

std::vector<gsl::not_null<std::shared_ptr<Voice>>>
  SoundEngine::getVoicesForAudioSource(Emitter* emitter, const std::shared_ptr<SoLoud::AudioSource>& audioSource) const
{
  const auto it1 = m_voices.find(emitter);
  if(it1 == m_voices.end())
    return {};

  const auto it2 = it1->second.find(audioSource);
  if(it2 == it1->second.end())
    return {};

  return it2->second;
}

bool SoundEngine::stop(const std::shared_ptr<SoLoud::AudioSource>& audioSource, Emitter* emitter)
{
  auto it1 = m_voices.find(emitter);
  if(it1 == m_voices.end())
    return false;

  const auto it2 = it1->second.find(audioSource);
  if(it2 == it1->second.end())
    return false;

  bool any = !it2->second.empty();
  for(const auto& src : it2->second)
  {
    src->stop();
  }

  it1->second.erase(it2);

  return any;
}

gsl::not_null<std::shared_ptr<Voice>> SoundEngine::play(const std::shared_ptr<SoLoud::AudioSource>& audioSource,
                                                        float pitch,
                                                        float volume,
                                                        Emitter* emitter)
{
  std::shared_ptr<Voice> voice;
  if(emitter != nullptr)
  {
    const auto pos = emitter->getPosition();
    voice = std::make_shared<Voice>(
      m_soLoud, audioSource, m_soLoud->play3d(*audioSource, pos.x, pos.y, pos.z, 0, 0, 0, volume, true));
    voice->init3dParams();
  }
  else
  {
    voice = std::make_shared<Voice>(m_soLoud, audioSource, m_soLoud->play(*audioSource, volume, 0, true));
  }
  Ensures(voice != nullptr);

  voice->setRelativePlaySpeed(pitch);
  m_voices[emitter][audioSource].emplace_back(voice);
  m_soLoud->update3dAudio();
  voice->play();
  return voice;
}

void SoundEngine::dropEmitter(Emitter* emitter)
{
  const auto it = m_voices.find(emitter);
  if(it == m_voices.end())
    return;

  for(const auto& [source, voices] : it->second)
    for(const auto& voice : voices)
      voice->stop();

  m_voices.erase(it);
}

SoundEngine::~SoundEngine()
{
  for(auto& emitter : m_emitters)
  {
    dropEmitter(emitter);
    emitter->m_engine = nullptr;
  }

  for(auto& listener : m_listeners)
    listener->m_engine = nullptr;

  m_soLoud->deinit();
}

gsl::not_null<std::shared_ptr<Voice>> SoundEngine::play(const std::shared_ptr<SoLoud::AudioSource>& audioSource,
                                                        float pitch,
                                                        float volume,
                                                        const glm::vec3& pos)
{
  auto voice = play(audioSource, pitch, volume, nullptr);
  voice->setPosition(pos);
  return voice;
}

void SoundEngine::reset()
{
  BOOST_LOG_TRIVIAL(debug) << "Resetting sound engine";
  m_soLoud->setGlobalVolume(0.0f);
  m_soLoud->stopAll();
  m_voices.clear();
  m_listener = nullptr;
  m_emitters.clear();
  m_listeners.clear();
}

SoundEngine::SoundEngine()
    : m_soLoud{std::make_shared<SoLoud::Soloud>()}
{
  Expects(m_soLoud->init() == SoLoud::SO_NO_ERROR);
  BOOST_LOG_TRIVIAL(info) << "SoLoud version " << m_soLoud->getVersion() << " initialized";
  BOOST_LOG_TRIVIAL(info) << "Backend " << m_soLoud->getBackendString() << " with " << m_soLoud->getBackendChannels()
                          << " channels at " << m_soLoud->getBackendSamplerate() << " Hz and buffer size "
                          << m_soLoud->getBackendBufferSize();

  m_soLoud->setGlobalVolume(0.0f);
  m_underwaterFilter.setParams(SoLoud::BiquadResonantFilter::LOWPASS, 250, 1);
}

Listener::~Listener()
{
  if(m_engine != nullptr)
  {
    m_engine->setListener(nullptr);
    m_engine->m_listeners.erase(this);
  }
}

Listener::Listener(const gsl::not_null<SoundEngine*>& engine)
    : m_engine{engine}
{
  m_engine->m_listeners.emplace(this);
}

Listener& Listener::operator=(const Listener& rhs)
{
  if(&rhs == this)
    return *this;

  if(m_engine != rhs.m_engine)
  {
    m_engine->m_listeners.erase(this);
    m_engine = rhs.m_engine;
    m_engine->m_listeners.emplace(this);
  }
  return *this;
}

Listener::Listener(Listener&& rhs)
    : m_engine{std::exchange(rhs.m_engine, nullptr)}
{
  m_engine->m_listeners.erase(&rhs);
  m_engine->m_listeners.emplace(this);
}

Listener& Listener::operator=(Listener&& rhs)
{
  m_engine->m_listeners.erase(this);
  m_engine->m_listeners.erase(&rhs);
  m_engine = std::exchange(rhs.m_engine, nullptr);
  m_engine->m_listeners.emplace(this);
  return *this;
}

Emitter::~Emitter()
{
  if(m_engine != nullptr)
  {
    m_engine->dropEmitter(this);
    m_engine->m_emitters.erase(this);
  }
}

Emitter::Emitter(const gsl::not_null<SoundEngine*>& engine)
    : m_engine{engine}
{
  m_engine->m_emitters.emplace(this);
}

Emitter& Emitter::operator=(const Emitter& rhs)
{
  if(&rhs == this)
    return *this;

  if(m_engine != rhs.m_engine)
  {
    m_engine->m_emitters.erase(this);
    m_engine = rhs.m_engine;
    m_engine->m_emitters.emplace(this);
  }
  return *this;
}

Emitter::Emitter(Emitter&& rhs)
    : m_engine{std::exchange(rhs.m_engine, nullptr)}
{
  m_engine->m_emitters.erase(&rhs);
  m_engine->m_emitters.emplace(this);
}

Emitter& Emitter::operator=(Emitter&& rhs)
{
  m_engine->m_emitters.erase(this);
  m_engine->m_emitters.erase(&rhs);
  m_engine = std::exchange(rhs.m_engine, nullptr);
  m_engine->m_emitters.emplace(this);
  return *this;
}
} // namespace audio
