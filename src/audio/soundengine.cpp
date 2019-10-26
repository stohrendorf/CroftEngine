#include "soundengine.h"

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
  else
  {
    BOOST_LOG_TRIVIAL(warning) << "No listener set";
  }

  for(auto& emitterMap : m_sources)
  {
    glm::vec3 pos;
    if(emitterMap.first != nullptr)
      pos = emitterMap.first->getPosition();

    for(auto& handleMap : emitterMap.second)
    {
      auto old = std::move(handleMap.second);
      std::copy_if(
        old.begin(), old.end(), std::back_inserter(handleMap.second), [](const auto& h) { return !h.expired(); });

      if(emitterMap.first == nullptr)
        continue;

      for(const auto& handle : handleMap.second)
      {
        if(const auto locked = handle.lock())
        {
          locked->setPosition(pos);
        }
      }
    }
  }
}

std::vector<gsl::not_null<std::shared_ptr<SourceHandle>>> SoundEngine::getSourcesForBuffer(Emitter* emitter,
                                                                                           size_t buffer) const
{
  const auto it1 = m_sources.find(emitter);
  if(it1 == m_sources.end())
    return {};

  const auto it2 = it1->second.find(buffer);
  if(it2 == it1->second.end())
    return {};

  std::vector<gsl::not_null<std::shared_ptr<SourceHandle>>> result;
  for(const auto& h : it2->second)
    if(auto locked = h.lock())
      result.emplace_back(std::move(locked));

  return result;
}

bool SoundEngine::stopBuffer(size_t bufferId, Emitter* emitter)
{
  auto it1 = m_sources.find(emitter);
  if(it1 == m_sources.end())
    return false;

  const auto it2 = it1->second.find(bufferId);
  if(it2 == it1->second.end())
    return false;

  bool any = false;
  for(const auto& src : it2->second)
  {
    if(const auto locked = src.lock())
    {
      locked->stop();
      any = true;
    }
  }

  it1->second.erase(it2);

  return any;
}

gsl::not_null<std::shared_ptr<SourceHandle>>
  SoundEngine::playBuffer(size_t bufferId, ALfloat pitch, ALfloat volume, Emitter* emitter)
{
  auto src = m_device.createSource();
  src->setBuffer(m_buffers.at(bufferId));
  src->setPitch(pitch);
  src->setGain(volume);
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

  m_sources[emitter][bufferId].emplace_back(src.get());

  return src;
}

void SoundEngine::dropEmitter(Emitter* emitter)
{
  const auto it = m_sources.find(emitter);
  if(it == m_sources.end())
    return;

  for(const auto& bufferMap : it->second)
    for(const auto& src : bufferMap.second)
      if(const auto locked = src.lock())
        locked->stop();

  m_sources.erase(it);
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

void SoundEngine::addWav(const gsl::not_null<const uint8_t*>& buffer)
{
  auto buf = std::make_shared<BufferHandle>();
  buf->fillFromWav(buffer.get());
  m_buffers.emplace_back(std::move(buf));
}

gsl::not_null<std::shared_ptr<SourceHandle>>
  SoundEngine::playBuffer(size_t bufferId, ALfloat pitch, ALfloat volume, const glm::vec3& pos)
{
  const auto handle = playBuffer(bufferId, pitch, volume, nullptr);
  handle->setPosition(pos);
  return handle;
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
}