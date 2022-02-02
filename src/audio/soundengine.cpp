#include "soundengine.h"

#include "buffervoice.h"
#include "device.h"
#include "emitter.h"
#include "listener.h"
#include "voice.h"

#include <AL/al.h>
#include <algorithm>
#include <boost/log/trivial.hpp>
#include <glm/fwd.hpp>
#include <gslu.h>
#include <iterator>
#include <utility>

namespace audio
{
void SoundEngine::update()
{
  m_device->update();
  if(m_listener != nullptr)
  {
    m_device->setListenerTransform(m_listener->getPosition(), m_listener->getFrontVector(), m_listener->getUpVector());
  }

  for(auto& [emitter, idsAndVoices] : m_voices)
  {
    glm::vec3 pos;
    if(emitter != nullptr)
      pos = emitter->getPosition();

    for(auto& [id, voices] : idsAndVoices)
    {
      auto old = std::move(voices);
      std::copy_if(old.begin(),
                   old.end(),
                   std::back_inserter(voices),
                   [](const auto& v)
                   {
                     return !v.expired();
                   });

      if(emitter == nullptr)
        continue;

      for(const auto& voice : voices)
      {
        if(const auto locked = voice.lock())
        {
          locked->setPosition(pos);
        }
      }
    }
  }
}

std::vector<gslu::nn_shared<Voice>> SoundEngine::getVoicesForBuffer(Emitter* emitter, size_t buffer) const
{
  const auto it1 = m_voices.find(emitter);
  if(it1 == m_voices.end())
    return {};

  const auto it2 = it1->second.find(buffer);
  if(it2 == it1->second.end())
    return {};

  std::vector<gslu::nn_shared<Voice>> result;
  for(const auto& v : it2->second)
    if(auto locked = v.lock())
      result.emplace_back(std::move(locked));

  return result;
}

bool SoundEngine::stopBuffer(size_t bufferId, const Emitter* emitter)
{
  auto it1 = m_voices.find(emitter);
  if(it1 == m_voices.end())
    return false;

  const auto it2 = it1->second.find(bufferId);
  if(it2 == it1->second.end())
    return false;

  bool any = false;
  for(const auto& v : it2->second)
  {
    if(const auto locked = v.lock())
    {
      locked->stop();
      any = true;
    }
  }

  it1->second.erase(it2);

  return any;
}

gslu::nn_shared<BufferVoice> SoundEngine::playBuffer(
  const gslu::nn_shared<BufferHandle>& buffer, size_t bufferId, ALfloat pitch, ALfloat volume, Emitter* emitter)
{
  auto v = gsl::make_shared<BufferVoice>(buffer);
  v->setPitch(pitch);
  v->setLocalGain(volume);
  if(emitter != nullptr)
    v->setPosition(emitter->getPosition());
  v->play();

  m_voices[emitter][bufferId].emplace_back(v.get());
  m_device->registerVoice(v);

  return v;
}

void SoundEngine::dropEmitter(const Emitter* emitter)
{
  const auto it = m_voices.find(emitter);
  if(it == m_voices.end())
    return;

  for(const auto& [id, handles] : it->second)
    for(const auto& src : handles)
      if(const auto locked = src.lock())
        locked->stop();

  m_voices.erase(it);
}

SoundEngine::SoundEngine()
    : m_device{std::make_unique<Device>()}
{
}

SoundEngine::~SoundEngine()
{
  reset();

  for(auto& emitter : m_emitters)
  {
    dropEmitter(emitter);
    emitter->m_engine = nullptr;
  }

  for(auto& listener : m_listeners)
    listener->m_engine = nullptr;
}

gslu::nn_shared<BufferVoice> SoundEngine::playBuffer(
  const gslu::nn_shared<BufferHandle>& buffer, size_t bufferId, ALfloat pitch, ALfloat volume, const glm::vec3& pos)
{
  auto voice = playBuffer(buffer, bufferId, pitch, volume, nullptr);
  voice->setPosition(pos);
  return voice;
}

void SoundEngine::reset()
{
  BOOST_LOG_TRIVIAL(debug) << "Resetting sound engine";
  m_device->reset();
  m_voices.clear();
  m_listener = nullptr;
  m_emitters.clear();
  m_listeners.clear();
}

void SoundEngine::setListenerGain(float gain)
{
  m_device->setListenerGain(gain);
}
} // namespace audio
