#pragma once

#include "alext.h"
#include "handle.h"
#include "utils.h"

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
