#include "soundengine.h"

#include "buffervoice.h"

#include <boost/log/trivial.hpp>
#include <glm/gtx/string_cast.hpp>

namespace audio
{
void SoundEngine::update()
{
  m_device.update();
  if(m_listener != nullptr)
  {
    m_device.setListenerTransform(m_listener->getPosition(), m_listener->getFrontVector(), m_listener->getUpVector());
  }

  for(auto& [emitter, idsAndVoices] : m_voices)
  {
    glm::vec3 pos;
    if(emitter != nullptr)
      pos = emitter->getPosition();

    for(auto& [id, voices] : idsAndVoices)
    {
      auto old = std::move(voices);
      std::copy_if(old.begin(), old.end(), std::back_inserter(voices), [](const auto& v) { return !v.expired(); });

      if(emitter == nullptr)
        continue;

      for(const auto& voice : voices)
      {
        if(const auto locked = voice.lock())
        {
          locked->getSource()->setPosition(pos);
        }
      }
    }
  }
}

std::vector<gsl::not_null<std::shared_ptr<Voice>>> SoundEngine::getVoicesForBuffer(Emitter* emitter,
                                                                                   size_t buffer) const
{
  const auto it1 = m_voices.find(emitter);
  if(it1 == m_voices.end())
    return {};

  const auto it2 = it1->second.find(buffer);
  if(it2 == it1->second.end())
    return {};

  std::vector<gsl::not_null<std::shared_ptr<Voice>>> result;
  for(const auto& v : it2->second)
    if(auto locked = v.lock())
      result.emplace_back(std::move(locked));

  return result;
}

bool SoundEngine::stopBuffer(size_t bufferId, Emitter* emitter)
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

gsl::not_null<std::shared_ptr<BufferVoice>> SoundEngine::playBuffer(
  const std::shared_ptr<BufferHandle>& buffer, size_t bufferId, ALfloat pitch, ALfloat volume, Emitter* emitter)
{
  auto src = m_device.createSourceHandle();
  auto v = std::make_shared<BufferVoice>(src, buffer);
  src->setPitch(pitch);
  v->setLocalGain(volume);
  if(emitter != nullptr)
  {
    src->setPosition(emitter->getPosition());
  }
  else
  {
    src->set(AL_SOURCE_RELATIVE, AL_TRUE);
    src->set(AL_POSITION, 0, 0, 0);
    src->set(AL_VELOCITY, 0, 0, 0);
  }

  src->play();

  m_voices[emitter][bufferId].emplace_back(v);

  return v;
}

void SoundEngine::dropEmitter(Emitter* emitter)
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

SoundEngine::~SoundEngine()
{
  for(auto& emitter : m_emitters)
  {
    dropEmitter(emitter);
    emitter->m_engine = nullptr;
  }

  for(auto& listener : m_listeners)
    listener->m_engine = nullptr;
}

gsl::not_null<std::shared_ptr<BufferVoice>> SoundEngine::playBuffer(
  const std::shared_ptr<BufferHandle>& buffer, size_t bufferId, ALfloat pitch, ALfloat volume, const glm::vec3& pos)
{
  auto voice = playBuffer(buffer, bufferId, pitch, volume, nullptr);
  voice->getSource()->setPosition(pos);
  return voice;
}

void SoundEngine::reset()
{
  BOOST_LOG_TRIVIAL(debug) << "Resetting sound engine";
  m_device.reset();
  m_voices.clear();
  m_listener = nullptr;
  m_emitters.clear();
  m_listeners.clear();
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

Listener::Listener(Listener&& rhs) noexcept
    : m_engine{std::exchange(rhs.m_engine, nullptr)}
{
  m_engine->m_listeners.erase(&rhs);
  m_engine->m_listeners.emplace(this);
}

Listener& Listener::operator=(Listener&& rhs) noexcept
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

Emitter::Emitter(Emitter&& rhs) noexcept
    : m_engine{std::exchange(rhs.m_engine, nullptr)}
{
  m_engine->m_emitters.erase(&rhs);
  m_engine->m_emitters.emplace(this);
}

Emitter& Emitter::operator=(Emitter&& rhs) noexcept
{
  m_engine->m_emitters.erase(this);
  m_engine->m_emitters.erase(&rhs);
  m_engine = std::exchange(rhs.m_engine, nullptr);
  m_engine->m_emitters.emplace(this);
  return *this;
}
} // namespace audio
