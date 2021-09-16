#pragma once

#include "handle.h"
#include "loadefx.h"

namespace audio
{
class FilterHandle final : public Handle
{
public:
  explicit FilterHandle()
      : Handle{alGenFilters, alIsFilter, alDeleteFilters}
  {
  }
};
} // namespace audio
