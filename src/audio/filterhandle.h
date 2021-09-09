#pragma once

#include "alext.h"
#include "handle.h"
#include "utils.h"

namespace audio
{
class FilterHandle final
{
  const Handle m_handle{alGenFilters, alIsFilter, alDeleteFilters};

public:
  explicit FilterHandle() = default;

  [[nodiscard]] ALuint get() const noexcept
  {
    return m_handle;
  }
};
} // namespace audio
