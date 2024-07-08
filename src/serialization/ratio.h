#pragma once

#include "serialization_fwd.h"

#include <cstdint>
#include <gsl/gsl-lite.hpp>
#include <ratio>

namespace serialization
{
template<typename TContext, std::intmax_t N, std::intmax_t D>
void serialize(const std::ratio<N, D>& data, const Serializer<TContext>& ser)
{
  ser.tag("ratio");
  ser["n"].node << data.num;
  ser["d"].node << data.den;
}

template<typename TContext, std::intmax_t N, std::intmax_t D>
void deserialize(std::ratio<N, D>& /*data*/, const Deserializer<TContext>& ser)
{
  ser.tag("ratio");
  std::intmax_t n, d;
  ser["n"].node >> n;
  ser["d"].node >> d;
  gsl_Assert(n == N);
  gsl_Assert(d == D);
}
} // namespace serialization
