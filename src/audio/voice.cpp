#include "voice.h"

#include <algorithm>
#include <utility>

namespace audio
{
void Voice::setGroupGain(const ALfloat groupGain)
{
  m_groupGain = groupGain;
  updateGain();
}

void Voice::setLocalGain(const ALfloat localGain)
{
  m_localGain = localGain;
  updateGain();
}

void Voice::play()
{
  if(m_source != nullptr)
  {
    m_source->play();
    m_startedPlaying = true;
  }
  m_paused = false;
  m_playStartTime = std::chrono::high_resolution_clock::now();
}

void Voice::rewind()
{
  if(m_source != nullptr)
    m_source->rewind();
  m_playStartTime = std::chrono::high_resolution_clock::now();
}

void Voice::pause()
{
  if(m_source != nullptr)
  {
    m_source->pause();
    m_startedPlaying = true;
  }
  m_paused = true;
  m_playStartTime.reset();
}

void Voice::stop()
{
  if(m_source != nullptr)
    m_source->stop();
  m_playStartTime.reset();
}

void Voice::setLooping(bool looping)
{
  if(m_source != nullptr)
    m_source->setLooping(looping);
  m_looping = looping;
}

void Voice::setPitch(ALfloat pitch)
{
  if(m_source != nullptr)
    m_source->setPitch(pitch);
}

void Voice::setPosition(const glm::vec3& position)
{
  if(m_source != nullptr)
    m_source->setPosition(position);
  m_position = position;
}

bool Voice::isStopped() const
{
  if(m_source != nullptr)
    return m_source->isStopped();
  if(!m_playStartTime.has_value())
    return true;
  if(m_looping)
    return false;

  const auto current = Clock::now() - *m_playStartTime;
  return current > getDuration();
}

bool Voice::isPositional() const
{
  return m_position.has_value();
}

bool Voice::hasSourceHandle() const
{
  return m_source != nullptr;
}

void Voice::associate(std::unique_ptr<SourceHandle>&& source)
{
  m_source = std::move(source);
  if(m_source == nullptr)
    return;

  m_source->setPitch(m_pitch);
  m_source->setLooping(m_looping);
  if(m_paused.has_value())
  {
    if(*m_paused)
      m_source->pause();
    else
      m_source->play();
  }
  if(m_position.has_value())
  {
    m_source->setPosition(*m_position);
  }
  else
  {
    m_source->set(AL_SOURCE_RELATIVE, AL_TRUE);
    m_source->setPosition({0, 0, 0});
    m_source->set(AL_VELOCITY, 0, 0, 0);
  }
  updateGain();
  m_startedPlaying = true;
}

void Voice::updateGain()
{
  if(m_source != nullptr)
    m_source->setGain(m_localGain * m_groupGain);
}

const std::optional<glm::vec3>& Voice::getPosition() const
{
  return m_position;
}

const std::unique_ptr<SourceHandle>& Voice::getSourceHandle() const
{
  return m_source;
}

bool Voice::done() const
{
  return m_startedPlaying && isStopped();
}

bool Voice::isPaused() const
{
  return m_paused.value_or(true);
}
} // namespace audio
