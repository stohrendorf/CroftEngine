#pragma once

#include <string>
#include <type_traits>

namespace qs
{
template<typename Unit, typename Type>
struct quantity;

namespace detail
{
template<typename>
struct is_quantity : std::false_type
{
};

template<typename Unit, typename Type>
struct is_quantity<quantity<Unit, Type>> : std::true_type
{
};
} // namespace detail

template<typename T>
concept Quantity = detail::is_quantity<T>::value;

template<typename Unit, typename Type>
struct quantity
{
  template<typename, typename>
  friend struct quantity;

  using unit = Unit;
  using type = Type;
  using self_type = quantity;

  template<typename T>
  using as_type = quantity<unit, T>;

  constexpr explicit quantity(type value = type{}) noexcept
      : value{value}
  {
  }

  constexpr quantity(const quantity& rhs) noexcept
      : value{rhs.value}
  {
  }

  constexpr quantity& operator=(const self_type& rhs) noexcept
  {
    value = rhs.value;
    return *this;
  }

  template<typename T>
  explicit quantity(T) = delete;

  [[nodiscard]] std::string toString() const
  {
    return std::to_string(value) + Unit::suffix();
  }

  template<typename T = type>
  [[nodiscard]] constexpr auto get() const noexcept(noexcept(static_cast<T>(this->value)))
  {
    return static_cast<T>(value);
  }

  template<typename T>
  [[nodiscard]] constexpr auto cast() const
  {
    if constexpr(!Quantity<T>)
    {
      return quantity<Unit, T>{get<T>()};
    }
    else
    {
      static_assert(std::is_same_v<typename T::unit, unit>, "Unit mismatch");
      return quantity<unit, typename T::type>{static_cast<T::type>(value)};
    }
  }

  template<typename T>
  constexpr auto& operator+=(const quantity<Unit, T>& r) noexcept(noexcept(this->value += r.value))
  {
    value += r.value;
    return *this;
  }

  template<typename T>
  constexpr auto& operator-=(const quantity<Unit, T>& r) noexcept(noexcept(this->value -= r.value))
  {
    value -= r.value;
    return *this;
  }

  template<typename T>
  constexpr auto& operator%=(const quantity<Unit, T>& r) noexcept(noexcept(this->value %= r.value))
  {
    value %= r.value;
    return *this;
  }

  template<typename T>
  constexpr auto& operator*=(const T& r) noexcept(noexcept(this->value *= r))
  {
    value *= r;
    return *this;
  }

  template<typename T>
  constexpr auto operator/=(const T& r) noexcept(noexcept(this->value /= r))
  {
    value /= r;
    return *this;
  }

  // unary operator +
  constexpr auto operator+() const noexcept
  {
    return *this;
  }

  // comparison operators
  template<typename T>
  constexpr bool operator<(const quantity<Unit, T>& r) const noexcept(noexcept(this->value < r.value))
  {
    return value < r.value;
  }

  template<typename T>
  constexpr bool operator<=(const quantity<Unit, T>& r) const noexcept(noexcept(this->value <= r.value))
  {
    return value <= r.value;
  }

  template<typename T>
  constexpr bool operator==(const quantity<Unit, T>& r) const noexcept(noexcept(this->value == r.value))
  {
    return value == r.value;
  }

  template<typename T>
  constexpr bool operator>(const quantity<Unit, T>& r) const noexcept(noexcept(this->value > r.value))
  {
    return value > r.value;
  }

  template<typename T>
  constexpr bool operator>=(const quantity<Unit, T>& r) const noexcept(noexcept(this->value >= r.value))
  {
    return value >= r.value;
  }

  template<typename T>
  constexpr bool operator!=(const quantity<Unit, T>& r) const noexcept(noexcept(this->value != r.value))
  {
    return value != r.value;
  }

private:
  type value{};
};

template<typename Unit, typename Type>
requires std::is_signed_v<Type> constexpr quantity<Unit, Type>
  operator-(quantity<Unit, Type> l) noexcept(noexcept(-l.get()))
{
  return quantity<Unit, Type>{static_cast<Type>(-l.get())};
}

template<typename Unit, typename Type>
constexpr quantity<Unit, Type> abs(const quantity<Unit, Type>& v) noexcept(noexcept(v.get() >= 0 ? v : -v))
{
  if constexpr(std::is_signed_v<Type>)
    return v.get() >= 0 ? v : -v;
  else
    return v;
}

template<typename Unit, typename Type>
[[nodiscard]] constexpr quantity<Unit, Type>
  lerp(const quantity<Unit, Type>& a, const quantity<Unit, Type>& b, const float alpha)
{
  return quantity<Unit, Type>{static_cast<Type>(a.get() * (1 - alpha) + b.get() * alpha)};
}
} // namespace qs
