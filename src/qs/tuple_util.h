#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

namespace qs::detail
{
template<typename... Args>
using tuple_concat_t = decltype(std::tuple_cat(std::declval<Args>()...));

template<typename T, typename A>
using drop_if_same_t = std::conditional_t<std::is_same_v<T, A>, std::tuple<>, std::tuple<A>>;

template<typename, typename>
struct drop_one;

template<typename T, typename A>
struct drop_one<T, std::tuple<A>>
{
  using type = drop_if_same_t<T, A>;
};

template<typename T>
struct drop_one<T, std::tuple<>>
{
  using type = std::tuple<>;
};

template<typename T, typename A, typename... Args>
struct drop_one<T, std::tuple<A, Args...>>
{
  using head = drop_if_same_t<T, A>;

  using type = std::conditional_t<std::is_same_v<head, std::tuple<>>,
                                  std::tuple<Args...>,
                                  tuple_concat_t<head, typename drop_one<T, std::tuple<Args...>>::type>>;
};

template<typename T, typename Tuple>
using drop_one_t = typename drop_one<T, Tuple>::type;

template<typename>
struct first_type;

template<typename T, typename... Args>
struct first_type<std::tuple<T, Args...>>
{
  using type = T;
};

template<typename T>
using first_t = typename first_type<T>::type;

template<typename>
struct except_first;

template<typename T, typename... Args>
struct except_first<std::tuple<T, Args...>>
{
  using type = std::tuple<Args...>;
};

template<typename T>
using except_first_t = typename except_first<T>::type;

template<typename Needles, typename Haystack>
struct drop_all_once
{
  using type = typename drop_all_once<except_first_t<Needles>, drop_one_t<first_t<Needles>, Haystack>>::type;
};

template<>
struct drop_all_once<std::tuple<>, std::tuple<>>
{
  using type = std::tuple<>;
};

template<typename Needles>
struct drop_all_once<Needles, std::tuple<>>
{
  using type = std::tuple<>;
};

template<typename Haystack>
struct drop_all_once<std::tuple<>, Haystack>
{
  using type = Haystack;
};

template<typename T, typename U>
using drop_all_once_t = typename drop_all_once<T, U>::type;

template<typename L, typename R>
struct symmetric_difference;

template<typename... L, typename... R>
struct symmetric_difference<std::tuple<L...>, std::tuple<R...>>
{
  using reduced_l = drop_all_once_t<std::tuple<R...>, std::tuple<L...>>;
  using reduced_r = drop_all_once_t<std::tuple<L...>, std::tuple<R...>>;
};
} // namespace qs::detail