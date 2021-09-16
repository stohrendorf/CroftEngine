#pragma once

#include "core.h"

#include <AL/al.h>
#include <chrono>
#include <glm/vec3.hpp>
#include <memory>
#include <optional>

namespace audio
{
class SourceHandle;

class Voice
{
public:
  explicit Voice() = default;
  virtual ~Voice();

  void setGroupGain(ALfloat groupGain);
  void setLocalGain(ALfloat localGain);

  void play();
  void rewind();
  void pause();
  void stop();
  virtual void setLooping(bool looping);

  void setPitch(ALfloat pitch);

  void setPosition(const glm::vec3& position);
  [[nodiscard]] const std::optional<glm::vec3>& getPosition() const;

  [[nodiscard]] bool isPaused() const;
  [[nodiscard]] bool isStopped() const;
  [[nodiscard]] bool isPositional() const;

  [[nodiscard]] bool hasSourceHandle() const;
  [[nodiscard]] const std::unique_ptr<SourceHandle>& getSourceHandle() const;

  virtual void associate(std::unique_ptr<SourceHandle>&& source);

  [[nodiscard]] bool done() const;

  [[nodiscard]] virtual Clock::duration getDuration() const = 0;

private:
  std::optional<std::chrono::high_resolution_clock::time_point> m_playStartTime{};
  ALfloat m_groupGain = 1.0f;
  ALfloat m_localGain = 1.0f;
  ALfloat m_pitch = 1.0f;
  std::optional<bool> m_paused{};
  bool m_looping = false;
  bool m_startedPlaying = false;
  std::optional<glm::vec3> m_position{};
  std::unique_ptr<SourceHandle> m_source;

  void updateGain();
};
} // namespace audio
