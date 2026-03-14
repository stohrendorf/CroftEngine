#pragma once

#include "serialization/serialization_fwd.h"
#include "tpl_helper.h"

#include <boost/throw_exception.hpp>
#include <concepts>
#include <gsl-lite/gsl-lite.hpp>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace core
{
template<std::unsigned_integral OffsetType, typename... DataTypes>
struct ContainerOffset
{
  static_assert(sizeof...(DataTypes) > 0, "Must provide at least one bound type");

  using offset_type = OffsetType;

  offset_type offset = 0;

  constexpr ContainerOffset() = default;

  explicit constexpr ContainerOffset(offset_type offset)
      : offset{offset}
  {
  }

  template<typename T>
  explicit ContainerOffset(T) = delete;

  template<typename T>
  [[nodiscard]] offset_type index() const
  {
    static_assert(tpl::is_one_of_v<T, DataTypes...>, "Can only use declared types for index conversion");
    if(offset % sizeof(T) != 0)
      BOOST_THROW_EXCEPTION(std::runtime_error("Offset not a multiple of element size"));

    return offset / sizeof(T);
  }

  template<typename T>
  [[nodiscard]] constexpr decltype(auto) from(std::vector<T>& v) const
  {
    return v.at(index<T>());
  }

  template<typename T>
  [[nodiscard]] constexpr decltype(auto) from(const std::vector<T>& v) const
  {
    return v.at(index<T>());
  }

  template<typename T>
  void operator=(T value) const = delete;

  auto& operator=(offset_type value) noexcept
  {
    offset = value;
    return *this;
  }
};

template<std::unsigned_integral IndexType, typename... DataTypes>
struct ContainerIndex
{
  static_assert(sizeof...(DataTypes) > 0, "Must provide at least one bound type");

  using index_type = IndexType;

  index_type index = 0;

  constexpr ContainerIndex() = default;

  explicit constexpr ContainerIndex(index_type index) noexcept
      : index{index}
  {
  }

  template<typename T>
  explicit ContainerIndex(T) = delete;

  template<typename T>
  [[nodiscard]] constexpr T& from(std::vector<T>& v) const requires(tpl::is_one_of_v<T, DataTypes...>)
  {
    return v.at(index);
  }

  template<typename T>
  [[nodiscard]] constexpr const T& from(const std::vector<T>& v) const requires(tpl::is_one_of_v<T, DataTypes...>)
  {
    return v.at(index);
  }

  template<typename T>
  [[nodiscard]] bool in(const std::vector<T>& v) const requires(tpl::is_one_of_v<T, DataTypes...>)
  {
    return index < v.size();
  }

  template<typename T>
  [[nodiscard]] bool exclusiveIn(const std::vector<T>& v) const noexcept requires(tpl::is_one_of_v<T, DataTypes...>)
  {
    return index <= v.size();
  }

  template<typename T>
  void operator+=(T delta) = delete;

  auto& operator+=(index_type delta)
  {
    if((index > 0) && (delta > std::numeric_limits<index_type>::max() - index))
      BOOST_THROW_EXCEPTION(std::out_of_range("Index addition causes overflow"));

    index += delta;
    return *this;
  }

  template<typename T>
  [[nodiscard]] auto operator+(const ContainerIndex<T, DataTypes...>& delta) const
  {
    return *this + delta.index;
  }

  template<typename T>
  [[nodiscard]] auto operator+(T delta) const
  {
    using LargerType = std::conditional_t<(sizeof(T) > sizeof(index_type)), T, index_type>;
    if((index > 0) && (delta > std::numeric_limits<LargerType>::max() - LargerType{index}))
      BOOST_THROW_EXCEPTION(std::out_of_range("Index addition causes overflow"));

    return ContainerIndex<LargerType, DataTypes...>{
      gsl_lite::narrow_cast<LargerType>(LargerType{index} + LargerType{delta})};
  }

  template<std::unsigned_integral T>
  // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature, misc-unconventional-assign-operator)
  ContainerIndex& operator=(T value) requires(sizeof(T) <= sizeof(IndexType))
  {
    index = value;
    return *this;
  }

  template<typename TContext>
  void serialize(const serialization::Serializer<TContext>& ser)
  {
    ser(S_NV("index", index));
  }

  template<typename TContext>
  static ContainerIndex create(const serialization::Serializer<TContext>& ser)
  {
    index_type tmp = 0;
    ser(S_NV("index", tmp));
    return {tmp};
  }
};
} // namespace core