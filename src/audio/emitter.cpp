#include "emitter.h"

#include "soundengine.h"

#include <gsl/gsl-lite.hpp>
#include <unordered_set>
#include <utility>

namespace audio
{
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
