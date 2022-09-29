#pragma once

#include "serialization_fwd.h"

#include <gsl/gsl-lite.hpp>
#include <ratio>

namespace serialization
{
template<typename TContext, std::intmax_t N, std::intmax_t D>
void serialize(std::ratio<N, D>& data, const Serializer<TContext>& ser)
{
  ser.tag("ratio");
  if(ser.loading)
  {
    std::intmax_t n, d;
    ser["n"].node >> n;
    ser["d"].node >> d;
    gsl_Assert(n == N);
    gsl_Assert(d == D);
  }
  else
  {
    ser["n"].node << data.num;
    ser["d"].node << data.den;
  }
}
} // namespace serialization
