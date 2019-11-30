#pragma once

#include "gsl-lite.hpp"
#include "mult_div.h"
#include "quantity.h"
#include "stream.h"

#define QS_DECLARE_QUANTITY(NAME, TYPE, SUFFIX) \
  struct _##NAME##_generated_unit               \
  {                                             \
    static gsl::czstring suffix()               \
    {                                           \
      return SUFFIX;                            \
    }                                           \
  };                                            \
  using NAME = ::qs::quantity<_##NAME##_generated_unit, TYPE>

#define QS_LITERAL_OP_ULL(TYPE, NAME)                               \
  constexpr TYPE operator"" NAME(unsigned long long value) noexcept \
  {                                                                 \
    return TYPE{static_cast<TYPE::type>(value)};                    \
  }

#define QS_LITERAL_OP_LD(TYPE, NAME)                         \
  constexpr TYPE operator"" NAME(long double value) noexcept \
  {                                                          \
    return TYPE{static_cast<TYPE::type>(value)};             \
  }

#define QS_COMBINE_UNITS(L, OP, R) decltype(std::declval<L>() OP std::declval<R>())
