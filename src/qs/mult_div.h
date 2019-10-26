#pragma once

#include "quantity.h"
#include "string_util.h"
#include "tuple_util.h"

#include <string>

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
  using _tmp = detail::tuple_drop_common<std::tuple<Top...>, std::tuple<Bottom...>>;

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

// operator *

// Follows: lhs = product_unit, rhs = (unit, product_unit, fraction_unit)

// product_unit * Unit
template<typename Type, typename... Units1, typename Unit2>
constexpr auto operator*(quantity<product_unit<Units1...>, Type> a, quantity<Unit2, Type> b)
{
  return quantity<product_unit<Units1..., Unit2>, Type>{a.get() * b.get()};
}

// product_unit * product_unit
template<typename Type, typename... Units1, typename... Units2>
constexpr auto operator*(quantity<product_unit<Units1...>, Type> a, quantity<product_unit<Units2...>, Type> b)
{
  return quantity<product_unit<Units1..., Units2...>, Type>{a.get() * b.get()};
}

// product_unit * fraction_unit
template<typename Type, typename... Units1, typename... Units2Top, typename... Units2Bottom>
constexpr auto operator*(quantity<product_unit<Units1...>, Type> a,
                         quantity<fraction_unit<std::tuple<Units2Top...>, std::tuple<Units2Bottom...>>, Type> b)
{
  return quantity<fraction_unit_t<std::tuple<Units1..., Units2Top...>, std::tuple<Units2Bottom...>>, Type>{a.get()
                                                                                                           * b.get()};
}

// Follows: lhs = unit, rhs = (unit, product_unit, fraction_unit)

// Unit * Unit
template<typename Type, typename Unit1, typename Unit2>
constexpr auto operator*(quantity<Unit1, Type> a, quantity<Unit2, Type> b)
{
  return quantity<product_unit<Unit1, Unit2>, Type>{a.get() * b.get()};
}

// Unit * product_unit
template<typename Type, typename Unit1, typename... Units2>
constexpr auto operator*(quantity<Unit1, Type> a, quantity<product_unit<Units2...>, Type> b)
{
  return quantity<product_unit<Unit1, Units2...>, Type>{a.get() * b.get()};
}

// Unit * fraction_unit
template<typename Type, typename Unit1, typename... Units2Top, typename... Units2Bottom>
constexpr auto operator*(quantity<Unit1, Type> a,
                         quantity<fraction_unit<std::tuple<Units2Top...>, std::tuple<Units2Bottom...>>, Type> b)
{
  return quantity<fraction_unit_t<std::tuple<Unit1, Units2Top...>, std::tuple<Units2Bottom...>>, Type>{a.get()
                                                                                                       * b.get()};
}

// Follows: lhs = fraction_unit, rhs = (unit, product_unit, fraction_unit)

// fraction_unit * unit
template<typename Type, typename... Units1Top, typename... Units1Bottom, typename Unit2>
constexpr auto operator*(quantity<fraction_unit<std::tuple<Units1Top...>, std::tuple<Units1Bottom...>>, Type> a,
                         quantity<Unit2, Type> b)
{
  return quantity<fraction_unit_t<std::tuple<Units1Top..., Unit2>, std::tuple<Units1Bottom...>>, Type>{a.get()
                                                                                                       * b.get()};
}

// fraction_unit * product_unit
template<typename Type, typename... Units1Top, typename... Units1Bottom, typename... Units2>
constexpr auto operator*(quantity<fraction_unit<std::tuple<Units1Top...>, std::tuple<Units1Bottom...>>, Type> a,
                         quantity<product_unit<Units2...>, Type> b)
{
  return quantity<fraction_unit_t<std::tuple<Units1Top..., Units2...>, std::tuple<Units1Bottom...>>, Type>{a.get()
                                                                                                           * b.get()};
}

// fraction_unit * fraction_unit
template<typename Type,
         typename... Units1Top,
         typename... Units1Bottom,
         typename... Units2Top,
         typename... Units2Bottom>
constexpr auto operator*(quantity<fraction_unit<std::tuple<Units1Top...>, std::tuple<Units1Bottom...>>, Type> a,
                         quantity<fraction_unit<std::tuple<Units2Top...>, std::tuple<Units2Bottom...>>, Type> b)
{
  return quantity<fraction_unit_t<std::tuple<Units1Top..., Units2Top...>, std::tuple<Units1Bottom..., Units2Bottom...>>,
                  Type>{a.get() * b.get()};
}

// operator /

// Follows: lhs = product_unit, rhs = (unit, product_unit, fraction_unit)

// product_unit / unit
template<typename Type,
         typename... Units1,
         typename Unit2,
         typename = detail::enable_if_not_same_t<product_unit<Units1...>, Unit2>>
constexpr auto operator/(quantity<product_unit<Units1...>, Type> a, quantity<Unit2, Type> b)
{
  return quantity<fraction_unit_t<std::tuple<Units1...>, std::tuple<Unit2>>, Type>{a.get() / b.get()};
}

// product_unit / product_unit
template<typename Type,
         typename... Units1,
         typename... Units2,
         typename = detail::enable_if_not_same_t<std::tuple<Units1...>, std::tuple<Units2...>>>
constexpr auto operator/(quantity<product_unit<Units1...>, Type> a, quantity<product_unit<Units2...>, Type> b)
{
  return quantity<fraction_unit_t<std::tuple<Units1...>, std::tuple<Units2...>>, Type>{a.get() / b.get()};
}

// product_unit / fraction_unit
template<typename Type, typename... Units1, typename... Units2Top, typename... Units2Bottom>
constexpr auto operator/(quantity<product_unit<Units1...>, Type> a,
                         quantity<fraction_unit<std::tuple<Units2Top...>, std::tuple<Units2Bottom...>>, Type> b)
{
  return quantity<fraction_unit_t<std::tuple<Units1..., Units2Bottom...>, std::tuple<Units2Top...>>, Type>{a.get()
                                                                                                           / b.get()};
}

// Follows: lhs = unit, rhs = (unit, product_unit, fraction_unit)

// unit / unit
template<typename Type, typename Unit1, typename Unit2>
constexpr auto operator/(quantity<Unit1, Type> a, quantity<Unit2, Type> b)
{
  return quantity<fraction_unit_t<std::tuple<Unit1>, std::tuple<Unit2>>, Type>{a.get() / b.get()};
}

// unit / product_unit
template<typename Type,
         typename Unit1,
         typename... Units2,
         typename = detail::enable_if_not_same_t<Unit1, product_unit<Units2...>>>
constexpr auto operator/(quantity<Unit1, Type> a, quantity<product_unit<Units2...>, Type> b)
{
  return quantity<fraction_unit_t<std::tuple<Unit1>, std::tuple<Units2...>>, Type>{a.get() / b.get()};
}

// unit / fraction_unit
template<typename Type, typename Unit1, typename... Units2Top, typename... Units2Bottom>
constexpr auto operator/(quantity<Unit1, Type> a,
                         quantity<fraction_unit<std::tuple<Units2Top...>, std::tuple<Units2Bottom...>>, Type> b)
{
  return quantity<fraction_unit_t<std::tuple<Unit1, Units2Bottom...>, std::tuple<Units2Top...>>, Type>{a.get()
                                                                                                       / b.get()};
}

// Follows: lhs = fraction_unit, rhs = (unit, product_unit, fraction_unit)

// fraction_unit / unit
template<typename Type, typename... Units1Top, typename... Units1Bottom, typename Unit2>
constexpr auto operator/(quantity<fraction_unit<std::tuple<Units1Top...>, std::tuple<Units1Bottom...>>, Type> a,
                         quantity<Unit2, Type> b)
{
  return quantity<fraction_unit_t<std::tuple<Units1Top...>, std::tuple<Units1Bottom..., Unit2>>, Type>{a.get()
                                                                                                       / b.get()};
}

// fraction_unit / product_unit
template<typename Type, typename... Units1Top, typename... Units1Bottom, typename... Units2>
constexpr auto operator/(quantity<fraction_unit<std::tuple<Units1Top...>, std::tuple<Units1Bottom...>>, Type> a,
                         quantity<product_unit<Units2...>, Type> b)
{
  return quantity<fraction_unit_t<std::tuple<Units1Top...>, std::tuple<Units1Bottom..., Units2...>>, Type>{a.get()
                                                                                                           / b.get()};
}

// fraction_unit / fraction_unit
template<typename Type,
         typename... Units1Top,
         typename... Units1Bottom,
         typename... Units2Top,
         typename... Units2Bottom>
constexpr auto operator/(quantity<fraction_unit<std::tuple<Units1Top...>, std::tuple<Units1Bottom...>>, Type> a,
                         quantity<fraction_unit<std::tuple<Units2Top...>, std::tuple<Units2Bottom...>>, Type> b)
{
  return quantity<fraction_unit_t<std::tuple<Units1Top..., Units2Bottom...>, std::tuple<Units1Bottom..., Units2Top...>>,
                  Type>{a.get() / b.get()};
}

template<typename Type, typename Unit>
constexpr auto operator/(quantity<Unit, Type> l, quantity<Unit, Type> r) noexcept
{
  return l.get() / r.get();
}

template<typename Type, typename Unit>
constexpr auto operator/(quantity<Unit, Type> l, Type r) noexcept
{
  return quantity<Unit, Type>{static_cast<Type>(l.get() / r)};
}

template<typename Type, typename Unit, typename T>
constexpr void operator/(quantity<Unit, Type> l, T r) = delete;

template<typename Type, typename Unit>
constexpr auto operator*(quantity<Unit, Type> l, Type r) noexcept
{
  return quantity<Unit, Type>{static_cast<Type>(l.get() * r)};
}

template<typename Type, typename Unit, typename T>
constexpr void operator*(quantity<Unit, Type> l, T r) = delete;

template<typename Type, typename Unit>
constexpr auto operator+(quantity<Unit, Type> l, quantity<Unit, Type> r) noexcept
{
  return quantity<Unit, Type>{static_cast<Type>(l.get() + r.get())};
}

template<typename Type, typename Unit>
constexpr auto operator-(quantity<Unit, Type> l, quantity<Unit, Type> r) noexcept
{
  return quantity<Unit, Type>{static_cast<Type>(l.get() - r.get())};
}

template<typename Type, typename Unit>
constexpr auto operator%(quantity<Unit, Type> l, quantity<Unit, Type> r) noexcept
{
  return quantity<Unit, Type>{static_cast<Type>(l.get() % r.get())};
}
} // namespace qs
