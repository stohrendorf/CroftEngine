#pragma once

#include <tuple>
#include <type_traits>

namespace qs::detail
{
template<typename... Args>
using tuple_concat_t = decltype(std::tuple_cat(std::declval<Args>()...));

template<typename T, typename A>
using drop_if_same_t = std::conditional_t<std::is_same<T, A>::value, std::tuple<>, std::tuple<A>>;

template<typename, typename>
struct drop_first;

template<typename T, typename A>
struct drop_first<T, std::tuple<A>>
{
  using type = drop_if_same_t<T, A>;
};

template<typename T>
struct drop_first<T, std::tuple<>>
{
  using type = std::tuple<>;
};

template<typename T, typename A, typename... Args>
struct drop_first<T, std::tuple<A, Args...>>
{
  using head = drop_if_same_t<T, A>;

  using type = std::conditional_t<std::is_same<head, std::tuple<>>::value,
                                  std::tuple<Args...>,
                                  tuple_concat_t<head, typename drop_first<T, std::tuple<Args...>>::type>>;
};

template<typename T, typename Tuple>
using drop_first_t = typename drop_first<T, Tuple>::type;

template<typename T, typename...>
using first_tuple_t = std::tuple<T>;

template<typename>
struct first_type;

template<typename T, typename... Args>
struct first_type<std::tuple<T, Args...>>
{
  using type = T;
};

template<typename T>
using first_type_t = typename first_type<T>::type;

template<typename>
struct except_first_tuple;

template<typename T, typename... Args>
struct except_first_tuple<std::tuple<T, Args...>>
{
  using type = std::tuple<Args...>;
};

template<typename T>
using except_first_tuple_t = typename except_first_tuple<T>::type;

template<typename Needles, typename Haystack>
struct drop_all_once
{
  using _needle0 = first_type_t<Needles>;
  using _reduced_haystack = drop_first_t<_needle0, Haystack>;

  using _reduced_needles = except_first_tuple_t<Needles>;
  using type = typename drop_all_once<_reduced_needles, _reduced_haystack>::type;
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
struct tuple_drop_common;

template<typename... L, typename... R>
struct tuple_drop_common<std::tuple<L...>, std::tuple<R...>>
{
  using reduced_l = drop_all_once_t<std::tuple<R...>, std::tuple<L...>>;
  using reduced_r = drop_all_once_t<std::tuple<L...>, std::tuple<R...>>;
};
}