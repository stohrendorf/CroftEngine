#pragma once

#include <glm/vec3.hpp>
#include <gsl-lite/gsl-lite.hpp>

namespace audio
{
class SoundEngine;

class Listener
{
  friend class SoundEngine;

public:
  explicit Listener(const gsl_lite::not_null<SoundEngine*>& engine);
  virtual ~Listener();

  Listener(const Listener& rhs)
      : Listener{gsl_lite::not_null{rhs.m_engine}}
  {
  }

  Listener() = delete;
  Listener& operator=(const Listener& rhs);
  Listener(Listener&& rhs) noexcept;
  Listener& operator=(Listener&&) noexcept;

  [[nodiscard]] virtual glm::vec3 getPosition() const = 0;
  [[nodiscard]] virtual glm::vec3 getFrontVector() const = 0;
  [[nodiscard]] virtual glm::vec3 getUpVector() const = 0;

private:
  mutable SoundEngine* m_engine = nullptr;
};
} // namespace audio