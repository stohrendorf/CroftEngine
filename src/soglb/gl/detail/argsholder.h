#pragma once

#include <tuple>
#include <utility>

namespace gl::detail
{
template<typename... Args>
struct ArgsHolder
{
  explicit ArgsHolder(Args&&... args)
      : args{std::forward<Args>(args)...}
  {
  }

  std::tuple<Args...> args;
};
} // namespace gl::detail

template<typename... Args>
auto set(Args&&... args)
{
  return gl::detail::ArgsHolder{std::forward<Args>(args)...};
}
