#pragma once

#include <type_traits>

namespace core::tpl
{
template<typename T, typename... Args>
constexpr bool is_one_of_v
  = (... || std::is_same_v<std::remove_cv<std::remove_reference_t<T>>, std::remove_cv<std::remove_reference_t<Args>>>);

template<typename... Args>
constexpr bool is_all_enum_v = (... && std::is_enum_v<Args>);
} // namespace core::tpl
