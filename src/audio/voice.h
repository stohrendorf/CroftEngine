#pragma once

#include "sourcehandle.h"

#include <gsl/gsl-lite.hpp>
#include <utility>

namespace audio
{
class Voice
{
public:
  explicit Voice(gsl::not_null<std::shared_ptr<SourceHandle>> source)
      : m_source{std::move(source)}
  {
  }

  virtual ~Voice() = default;

  void setGroupGain(const ALfloat groupGain)
  {
    m_groupGain = groupGain;
    updateGain();
  }

  void setLocalGain(const ALfloat localGain)
  {
    m_localGain = localGain;
    updateGain();
  }

  void play()
  {
    m_source->play();
  }

  void stop()
  {
    m_source->stop();
  }

  [[nodiscard]] bool isPaused() const
  {
    return m_source->isPaused();
  }

  [[nodiscard]] virtual bool isStopped() const
  {
    return m_source->isStopped();
  }

  [[nodiscard]] const auto& getSource() const
  {
    return m_source;
  }

private:
  float m_groupGain = 1.0f;
  float m_localGain = 1.0f;
  gsl::not_null<std::shared_ptr<SourceHandle>> m_source;

  void updateGain()
  {
    m_source->set(AL_GAIN, std::clamp(m_localGain * m_groupGain, 0.0f, 1.0f));
  }
};
} // namespace audio