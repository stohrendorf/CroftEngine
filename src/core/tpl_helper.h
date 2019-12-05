#pragma once

#include <type_traits>

namespace core::tpl
{
template<typename T, typename... Args>
struct contains;

template<typename T>
struct contains<T>
{
  static constexpr bool value = false;
};

template<typename T, typename... Args>
struct contains<T, T, Args...>
{
  static constexpr bool value = true;
};

template<typename T, typename A, typename... Args>
struct contains<T, A, Args...>
{
  static constexpr bool value = contains<T, Args...>::value;
};

template<typename T, typename... Args>
constexpr bool contains_v
  = contains<std::remove_cv<std::remove_reference_t<T>>, std::remove_cv<std::remove_reference_t<Args>>...>::value;

template<typename... Args>
struct is_all_enum;

template<>
struct is_all_enum<>
{
  static constexpr bool value = true;
};

template<typename T>
struct is_all_enum<T>
{
  static constexpr bool value = std::is_enum_v<T>;
};

template<typename T, typename... Args>
struct is_all_enum<T, Args...>
{
  static constexpr bool value = std::is_enum_v<T> && is_all_enum<Args...>::value;
};

template<typename... Args>
constexpr bool is_all_enum_v = is_all_enum<Args...>::value;
} // namespace tpl
