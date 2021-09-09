#pragma once

#include <glm/vec3.hpp>
#include <gsl/gsl-lite.hpp>

namespace audio
{
class SoundEngine;

class Emitter
{
  friend class SoundEngine;

public:
  explicit Emitter(const gsl::not_null<SoundEngine*>& engine);
  virtual ~Emitter();

  Emitter(const Emitter& rhs)
      : Emitter{rhs.m_engine}
  {
  }

  Emitter() = delete;

  Emitter& operator=(const Emitter& rhs);
  Emitter(Emitter&& rhs) noexcept;
  Emitter& operator=(Emitter&& rhs) noexcept;
  [[nodiscard]] virtual glm::vec3 getPosition() const = 0;

private:
  mutable SoundEngine* m_engine = nullptr;
};
} // namespace audio
