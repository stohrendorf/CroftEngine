#pragma once

#include "mult_div.h" // IWYU pragma: export
#include "quantity.h" // IWYU pragma: export
#include "stream.h"   // IWYU pragma: export

#include <gsl-lite/gsl-lite.hpp>
#include <type_traits>

// NOLINTNEXTLINE(bugprone-reserved-identifier)
#define _QS_PAREN_WRAPPER(value) value
// NOLINTNEXTLINE(bugprone-reserved-identifier)
#define _QS_CONCAT(a, b) a##b

#define QS_DECLARE_QUANTITY(NAME, TYPE, SUFFIX) \
  struct _##NAME##_generated_unit               \
  {                                             \
    static gsl_lite::czstring suffix() noexcept \
    {                                           \
      return SUFFIX;                            \
    }                                           \
  };                                            \
  using _QS_PAREN_WRAPPER(NAME) = ::qs::quantity<_##NAME##_generated_unit, _QS_PAREN_WRAPPER(TYPE)>

#define QS_LITERAL_OP_ULL(TYPE, NAME)                                                               \
  constexpr _QS_PAREN_WRAPPER(TYPE) _QS_CONCAT(operator"", NAME)(unsigned long long value) noexcept \
  {                                                                                                 \
    return _QS_PAREN_WRAPPER(TYPE){static_cast<_QS_PAREN_WRAPPER(TYPE)::type>(value)};              \
  }

#define QS_LITERAL_OP_LD(TYPE, NAME)                                                         \
  constexpr _QS_PAREN_WRAPPER(TYPE) _QS_CONCAT(operator"", NAME)(long double value) noexcept \
  {                                                                                          \
    return _QS_PAREN_WRAPPER(TYPE){static_cast<_QS_PAREN_WRAPPER(TYPE)::type>(value)};       \
  }

namespace qs::detail
{
template<typename T>
constexpr auto quantity_declval() -> T requires Quantity<T>;
}

#define QS_COMBINE_UNITS(L, OP, R)                                \
  decltype(::qs::detail::quantity_declval<_QS_PAREN_WRAPPER(L)>() \
             _QS_PAREN_WRAPPER(OP)::qs::detail::quantity_declval<_QS_PAREN_WRAPPER(R)>())
