#pragma once

#include "ratio.h"
#include "serialization_fwd.h"

#include <chrono>

namespace serialization
{
template<typename TContext, typename Rep, typename Period>
void serialize(std::chrono::duration<Rep, Period>& data, const Serializer<TContext>& ser)
{
  ser.tag("duration");
  if(ser.loading)
  {
    Rep n{};
    Period p{};
    ser(S_NV("count", n), S_NV("period", p));
    data = std::chrono::duration<Rep, Period>{n};
  }
  else
  {
    Rep n{data.count()};
    Period p{};
    ser(S_NV("count", n), S_NV("period", p));
  }
}

template<typename TContext, typename Rep, typename Period>
std::chrono::duration<Rep, Period> create(const TypeId<std::chrono::duration<Rep, Period>>&,
                                          const Serializer<TContext>& ser)
{
  std::chrono::duration<Rep, Period> tmp;
  serialize(tmp, ser);
  return tmp;
}
} // namespace serialization
