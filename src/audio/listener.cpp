#include "listener.h"

#include "soundengine.h"

#include <gsl/gsl-lite.hpp>
#include <unordered_set>
#include <utility>

namespace audio
{
Listener::~Listener()
{
  if(m_engine == nullptr)
    return;

  m_engine->setListener(nullptr);
  m_engine->m_listeners.erase(this);
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
    if(m_engine != nullptr)
      m_engine->m_listeners.erase(this);
    m_engine = rhs.m_engine;
    if(m_engine != nullptr)
      m_engine->m_listeners.emplace(this);
  }
  return *this;
}

Listener::Listener(Listener&& rhs) noexcept
    : m_engine{std::exchange(rhs.m_engine, nullptr)}
{
  if(m_engine == nullptr)
    return;

  m_engine->m_listeners.erase(&rhs);
  m_engine->m_listeners.emplace(this);
}

Listener& Listener::operator=(Listener&& rhs) noexcept
{
  if(m_engine != nullptr)
  {
    m_engine->m_listeners.erase(this);
    m_engine->m_listeners.erase(&rhs);
  }
  m_engine = std::exchange(rhs.m_engine, nullptr);
  if(m_engine != nullptr)
    m_engine->m_listeners.emplace(this);
  return *this;
}
} // namespace audio
