#pragma once

#include <string>

namespace qs::detail
{
template<typename U, typename... Units>
struct unit_suffix_helper
{
  static std::string suffix()
  {
    return std::string(U::suffix()) + "*" + unit_suffix_helper<Units...>::suffix();
  }
};

template<typename U>
struct unit_suffix_helper<U>
{
  static std::string suffix()
  {
    return U::suffix();
  }
};

template<typename... Units>
inline std::string suffix()
{
  return unit_suffix_helper<Units...>::suffix();
}
} // namespace qs::detail
