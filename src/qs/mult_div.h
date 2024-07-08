#pragma once

// IWYU pragma: private, include "quantity.h"

#include "quantity.h"
#include "string_util.h"
#include "tuple_util.h"

#include <cmath>
#include <string>
#include <tuple>
#include <type_traits>

namespace qs
{
namespace detail
{
template<typename A, typename B>
using enable_if_not_same_t = std::enable_if_t<!std::is_same_v<A, B>, bool>;
}

template<typename... Units>
struct product_unit
{
  static std::string suffix()
  {
    return detail::suffix<Units...>();
  }
};

template<typename, typename>
struct fraction_unit;

namespace detail
{
template<typename...>
struct flatten_tuple_product;

template<typename... Args>
struct flatten_tuple_product<std::tuple<Args...>>
{
  using type = product_unit<Args...>;
};

template<typename T>
struct flatten_tuple_product<std::tuple<T>>
{
  using type = T;
};

template<typename T>
using flatten_tuple_product_t = typename flatten_tuple_product<T>::type;

template<typename, typename>
struct flattener;

template<typename... Top, typename... Bottom>
struct flattener<std::tuple<Top...>, std::tuple<Bottom...>>
{
  using type = fraction_unit<std::tuple<Top...>, std::tuple<Bottom...>>;
};
template<typename... Top>
struct flattener<std::tuple<Top...>, std::tuple<>>
{
  using type = product_unit<Top...>;
};

template<typename Top>
struct flattener<std::tuple<Top>, std::tuple<>>
{
  using type = Top;
};

template<typename A, typename B>
using flattener_t = typename flattener<A, B>::type;
} // namespace detail

template<typename... Top, typename... Bottom>
struct fraction_unit<std::tuple<Top...>, std::tuple<Bottom...>>
{
  using _tmp = detail::symmetric_difference<std::tuple<Top...>, std::tuple<Bottom...>>;

  using top = typename _tmp::reduced_l;
  using bottom = typename _tmp::reduced_r;
  using type = detail::flattener_t<top, bottom>;

  static std::string suffix()
  {
    return std::string("(") + detail::flatten_tuple_product_t<top>::suffix() + ")/("
           + detail::flatten_tuple_product_t<bottom>::suffix() + ")";
  }
};

template<typename A, typename B>
using fraction_unit_t = typename fraction_unit<A, B>::type;

// NOLINTNEXTLINE(bugprone-reserved-identifier)
#define _QS_COMBINE_TS(T1, OP, T2) decltype(std::declval<T1>() OP std::declval<T2>())

// operator *

template<typename Unit, typename Type1, typename Type2>
constexpr auto operator*(const Type1& l, const quantity<Unit, Type2>& r) noexcept(noexcept(l * r.get()))
{
  return quantity<Unit, _QS_COMBINE_TS(Type1, *, Type2)>{l * r.get()};
}

// Follows: lhs = product_unit, rhs = (unit, product_unit, fraction_unit)

// product_unit * Unit
template<typename Type1, typename Type2, typename... Units1, typename Unit2>
constexpr auto operator*(const quantity<product_unit<Units1...>, Type1>& a,
                         const quantity<Unit2, Type2>& b) noexcept(noexcept(a.get() * b.get()))
{
  return quantity<product_unit<Units1..., Unit2>, _QS_COMBINE_TS(Type1, *, Type2)>{a.get() * b.get()};
}

// product_unit * product_unit
template<typename Type1, typename Type2, typename... Units1, typename... Units2>
constexpr auto operator*(const quantity<product_unit<Units1...>, Type1>& a,
                         const quantity<product_unit<Units2...>, Type2>& b) noexcept(noexcept(a.get() * b.get()))
{
  return quantity<product_unit<Units1..., Units2...>, _QS_COMBINE_TS(Type1, *, Type2)>{a.get() * b.get()};
}

// product_unit * fraction_unit
template<typename Type1, typename Type2, typename... Units1, typename... Units2Top, typename... Units2Bottom>
constexpr auto operator*(const quantity<product_unit<Units1...>, Type1>& a,
                         const quantity<fraction_unit<std::tuple<Units2Top...>, std::tuple<Units2Bottom...>>, Type2>&
                           b) noexcept(noexcept(a.get() * b.get()))
{
  return quantity<fraction_unit_t<std::tuple<Units1..., Units2Top...>, std::tuple<Units2Bottom...>>,
                  _QS_COMBINE_TS(Type1, *, Type2)>{a.get() * b.get()};
}

// Follows: lhs = unit, rhs = (unit, product_unit, fraction_unit)

// Unit * Unit
template<typename Type1, typename Type2, typename Unit1, typename Unit2>
constexpr auto operator*(const quantity<Unit1, Type1>& a,
                         const quantity<Unit2, Type2>& b) noexcept(noexcept(a.get() * b.get()))
{
  return quantity<product_unit<Unit1, Unit2>, _QS_COMBINE_TS(Type1, *, Type2)>{a.get() * b.get()};
}

// Unit * product_unit
template<typename Type1, typename Type2, typename Unit1, typename... Units2>
constexpr auto operator*(const quantity<Unit1, Type1>& a,
                         const quantity<product_unit<Units2...>, Type2>& b) noexcept(noexcept(a.get() * b.get()))
{
  return quantity<product_unit<Unit1, Units2...>, _QS_COMBINE_TS(Type1, *, Type2)>{a.get() * b.get()};
}

// Unit * fraction_unit
template<typename Type1, typename Type2, typename Unit1, typename... Units2Top, typename... Units2Bottom>
constexpr auto operator*(const quantity<Unit1, Type1>& a,
                         const quantity<fraction_unit<std::tuple<Units2Top...>, std::tuple<Units2Bottom...>>, Type2>&
                           b) noexcept(noexcept(a.get() * b.get()))
{
  return quantity<fraction_unit_t<std::tuple<Unit1, Units2Top...>, std::tuple<Units2Bottom...>>,
                  _QS_COMBINE_TS(Type1, *, Type2)>{a.get() * b.get()};
}

// Follows: lhs = fraction_unit, rhs = (unit, product_unit, fraction_unit)

// fraction_unit * unit
template<typename Type1, typename Type2, typename... Units1Top, typename... Units1Bottom, typename Unit2>
constexpr auto operator*(const quantity<fraction_unit<std::tuple<Units1Top...>, std::tuple<Units1Bottom...>>, Type1>& a,
                         const quantity<Unit2, Type2>& b) noexcept(noexcept(a.get() * b.get()))
{
  return quantity<fraction_unit_t<std::tuple<Units1Top..., Unit2>, std::tuple<Units1Bottom...>>,
                  _QS_COMBINE_TS(Type1, *, Type2)>{a.get() * b.get()};
}

// fraction_unit * product_unit
template<typename Type1, typename Type2, typename... Units1Top, typename... Units1Bottom, typename... Units2>
constexpr auto operator*(const quantity<fraction_unit<std::tuple<Units1Top...>, std::tuple<Units1Bottom...>>, Type1>& a,
                         const quantity<product_unit<Units2...>, Type2>& b) noexcept(noexcept(a.get() * b.get()))
{
  return quantity<fraction_unit_t<std::tuple<Units1Top..., Units2...>, std::tuple<Units1Bottom...>>,
                  _QS_COMBINE_TS(Type1, *, Type2)>{a.get() * b.get()};
}

// fraction_unit * fraction_unit
template<typename Type1,
         typename Type2,
         typename... Units1Top,
         typename... Units1Bottom,
         typename... Units2Top,
         typename... Units2Bottom>
constexpr auto operator*(const quantity<fraction_unit<std::tuple<Units1Top...>, std::tuple<Units1Bottom...>>, Type1>& a,
                         const quantity<fraction_unit<std::tuple<Units2Top...>, std::tuple<Units2Bottom...>>, Type2>&
                           b) noexcept(noexcept(a.get() * b.get()))
{
  return quantity<fraction_unit_t<std::tuple<Units1Top..., Units2Top...>, std::tuple<Units1Bottom..., Units2Bottom...>>,
                  _QS_COMBINE_TS(Type1, *, Type2)>{a.get() * b.get()};
}

// operator /

// Follows: lhs = product_unit, rhs = (unit, product_unit, fraction_unit)

// product_unit / unit
template<typename Type1,
         typename Type2,
         typename... Units1,
         typename Unit2,
         typename = detail::enable_if_not_same_t<product_unit<Units1...>, Unit2>>
constexpr auto operator/(const quantity<product_unit<Units1...>, Type1>& a,
                         const quantity<Unit2, Type2>& b) noexcept(noexcept(a.get() / b.get()))
{
  return quantity<fraction_unit_t<std::tuple<Units1...>, std::tuple<Unit2>>, _QS_COMBINE_TS(Type1, /, Type2)>{
    a.get() / b.get()};
}

// product_unit / product_unit
template<typename Type1,
         typename Type2,
         typename... Units1,
         typename... Units2,
         typename = detail::enable_if_not_same_t<std::tuple<Units1...>, std::tuple<Units2...>>>
constexpr auto operator/(const quantity<product_unit<Units1...>, Type1>& a,
                         const quantity<product_unit<Units2...>, Type2>& b) noexcept(noexcept(a.get() / b.get()))
{
  return quantity<fraction_unit_t<std::tuple<Units1...>, std::tuple<Units2...>>, _QS_COMBINE_TS(Type1, /, Type2)>{
    a.get() / b.get()};
}

// product_unit / fraction_unit
template<typename Type1, typename Type2, typename... Units1, typename... Units2Top, typename... Units2Bottom>
constexpr auto operator/(const quantity<product_unit<Units1...>, Type1>& a,
                         const quantity<fraction_unit<std::tuple<Units2Top...>, std::tuple<Units2Bottom...>>, Type2>&
                           b) noexcept(noexcept(a.get() / b.get()))
{
  return quantity<fraction_unit_t<std::tuple<Units1..., Units2Bottom...>, std::tuple<Units2Top...>>,
                  _QS_COMBINE_TS(Type1, /, Type2)>{a.get() / b.get()};
}

// Follows: lhs = unit, rhs = (unit, product_unit, fraction_unit)

// unit / unit
template<typename Type1, typename Type2, typename Unit1, typename Unit2>
constexpr auto operator/(const quantity<Unit1, Type1>& a,
                         const quantity<Unit2, Type2>& b) noexcept(noexcept(a.get() / b.get()))
{
  return quantity<fraction_unit_t<std::tuple<Unit1>, std::tuple<Unit2>>, _QS_COMBINE_TS(Type1, /, Type2)>{a.get()
                                                                                                          / b.get()};
}

// unit / product_unit
template<typename Type1,
         typename Type2,
         typename Unit1,
         typename... Units2,
         typename = detail::enable_if_not_same_t<Unit1, product_unit<Units2...>>>
constexpr auto operator/(const quantity<Unit1, Type1>& a,
                         const quantity<product_unit<Units2...>, Type2>& b) noexcept(noexcept(a.get() / b.get()))
{
  return quantity<fraction_unit_t<std::tuple<Unit1>, std::tuple<Units2...>>, _QS_COMBINE_TS(Type1, /, Type2)>{
    a.get() / b.get()};
}

// unit / fraction_unit
template<typename Type1, typename Type2, typename Unit1, typename... Units2Top, typename... Units2Bottom>
constexpr auto operator/(const quantity<Unit1, Type1>& a,
                         const quantity<fraction_unit<std::tuple<Units2Top...>, std::tuple<Units2Bottom...>>, Type2>&
                           b) noexcept(noexcept(a.get() / b.get()))
{
  return quantity<fraction_unit_t<std::tuple<Unit1, Units2Bottom...>, std::tuple<Units2Top...>>,
                  _QS_COMBINE_TS(Type1, /, Type2)>{a.get() / b.get()};
}

// Follows: lhs = fraction_unit, rhs = (unit, product_unit, fraction_unit)

// fraction_unit / unit
template<typename Type1, typename Type2, typename... Units1Top, typename... Units1Bottom, typename Unit2>
constexpr auto operator/(const quantity<fraction_unit<std::tuple<Units1Top...>, std::tuple<Units1Bottom...>>, Type1>& a,
                         const quantity<Unit2, Type2>& b) noexcept(noexcept(a.get() / b.get()))
{
  return quantity<fraction_unit_t<std::tuple<Units1Top...>, std::tuple<Units1Bottom..., Unit2>>,
                  _QS_COMBINE_TS(Type1, /, Type2)>{a.get() / b.get()};
}

// fraction_unit / product_unit
template<typename Type1, typename Type2, typename... Units1Top, typename... Units1Bottom, typename... Units2>
constexpr auto operator/(const quantity<fraction_unit<std::tuple<Units1Top...>, std::tuple<Units1Bottom...>>, Type1>& a,
                         const quantity<product_unit<Units2...>, Type2>& b) noexcept(noexcept(a.get() / b.get()))
{
  return quantity<fraction_unit_t<std::tuple<Units1Top...>, std::tuple<Units1Bottom..., Units2...>>,
                  _QS_COMBINE_TS(Type1, /, Type2)>{a.get() / b.get()};
}

// fraction_unit / fraction_unit
template<typename Type1,
         typename Type2,
         typename... Units1Top,
         typename... Units1Bottom,
         typename... Units2Top,
         typename... Units2Bottom>
constexpr auto operator/(const quantity<fraction_unit<std::tuple<Units1Top...>, std::tuple<Units1Bottom...>>, Type1>& a,
                         const quantity<fraction_unit<std::tuple<Units2Top...>, std::tuple<Units2Bottom...>>, Type2>&
                           b) noexcept(noexcept(a.get() / b.get()))
{
  return quantity<fraction_unit_t<std::tuple<Units1Top..., Units2Bottom...>, std::tuple<Units1Bottom..., Units2Top...>>,
                  _QS_COMBINE_TS(Type1, /, Type2)>{a.get() / b.get()};
}

template<typename Type1, typename Type2, typename Unit>
constexpr auto operator/(const quantity<Unit, Type1>& a,
                         const quantity<Unit, Type2>& b) noexcept(noexcept(a.get() / b.get()))
{
  return a.get() / b.get();
}

template<typename Type1, typename Type2, typename Unit>
constexpr auto operator/(const quantity<Unit, Type1>& a, const Type2& b) noexcept(noexcept(a.get() / b))
{
  return quantity<Unit, _QS_COMBINE_TS(Type1, /, Type2)>{a.get() / b};
}

template<typename Type1, typename Type2, typename Unit>
constexpr auto operator*(const quantity<Unit, Type1>& a, const Type2& b) noexcept(noexcept(a.get() * b))
{
  return quantity<Unit, _QS_COMBINE_TS(Type1, *, Type2)>{a.get() * b};
}

template<typename Type1, typename Type2, typename Unit>
constexpr auto operator+(const quantity<Unit, Type1>& a,
                         const quantity<Unit, Type2>& b) noexcept(noexcept(a.get() + b.get()))
{
  return quantity<Unit, _QS_COMBINE_TS(Type1, +, Type2)>{a.get() + b.get()};
}

template<typename Type1, typename Type2, typename Unit>
constexpr auto operator-(const quantity<Unit, Type1>& a,
                         const quantity<Unit, Type2>& b) noexcept(noexcept(a.get() - b.get()))
{
  return quantity<Unit, _QS_COMBINE_TS(Type1, -, Type2)>{a.get() - b.get()};
}

template<typename Type1, typename Type2, typename Unit>
constexpr auto operator%(const quantity<Unit, Type1>& a,
                         const quantity<Unit, Type2>& b) noexcept(noexcept(a.get() % b.get()))
{
  return quantity<Unit, _QS_COMBINE_TS(Type1, %, Type2)>{a.get() % b.get()};
}

template<typename Unit>
constexpr auto operator%(const quantity<Unit, float>& a,
                         const quantity<Unit, float>& b) noexcept(noexcept(std::fmod(a.get(), b.get())))
{
  return quantity<Unit, float>{static_cast<float>(std::fmod(a.get(), b.get()))};
}

#undef _QS_COMBINE_TS
} // namespace qs
