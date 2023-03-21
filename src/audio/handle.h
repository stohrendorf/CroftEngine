#pragma once

#include "utils.h"

#include <AL/al.h> // IWYU pragma: export
#include <gsl/gsl-lite.hpp>

namespace audio
{
class Handle
{
public:
  using Allocator = void(ALsizei, ALuint*);
  using Deleter = void(ALsizei, const ALuint*);
  using Test = ALboolean(ALuint);

  explicit Handle(Allocator* allocator, Test* test, Deleter* deleter)
      : m_handle{createHandle(allocator, test)}
      , m_deleter{deleter}
  {
  }

  virtual ~Handle()
  {
    AL_ASSERT(m_deleter(1, &m_handle));
  }

  Handle(const Handle&) = delete;
  Handle(Handle&&) = delete;
  void operator=(const Handle&) = delete;
  void operator=(Handle&&) = delete;

  // NOLINTNEXTLINE(google-explicit-constructor)
  operator ALuint() const noexcept
  {
    return m_handle;
  }

private:
  [[nodiscard]] static ALuint createHandle(Allocator* allocator, Test* test)
  {
    ALuint handle;
    AL_ASSERT(allocator(1, &handle));
    gsl_Ensures(test(handle) == AL_TRUE);
    return handle;
  }

  ALuint m_handle;
  Deleter* m_deleter;
};
} // namespace audio
