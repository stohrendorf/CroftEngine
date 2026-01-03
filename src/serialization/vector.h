#pragma once

#include "access.h" // IWYU pragma: keep
#include "serialization.h"

#include <functional>
#include <gsl-lite/gsl-lite.hpp>
#include <vector>

namespace serialization
{
template<typename T, typename TContext>
void serialize(const std::vector<T>& data, const Serializer<TContext>& ser)
{
  ser.tag("vector");
  ser.node |= ryml::SEQ;
  for(auto& element : data)
  {
    const auto tmp = ser.newChild();
    access::dispatch(element, tmp);
  }
}

template<typename T, typename TContext>
void deserialize(std::vector<T>& data, const Deserializer<TContext>& ser)
{
  ser.tag("vector");
  gsl_Expects(ser.node.is_seq());
  data = std::vector<T>();
  data.reserve(ser.node.num_children());
  std::transform(ser.node.begin(),
                 ser.node.end(),
                 std::back_inserter(data),
                 [&ser](const ryml::ConstNodeRef& element)
                 {
                   return access::dispatch<T>(ser.withNode(element));
                 });
}

template<typename T>
struct DeserializingFrozenVector
{
  std::reference_wrapper<std::vector<T>> vec;

  explicit DeserializingFrozenVector(std::reference_wrapper<std::vector<T>>&& vec)
      : vec{std::move(vec)}
  {
  }

  template<typename TContext>
  void deserialize(const Deserializer<TContext>& ser) const
  {
    gsl_Expects(ser.node.num_children() == vec.get().size());
    auto it = vec.get().begin();
    for(const auto& element : ser.node.children())
    {
      access::dispatch(*it++, ser.withNode(element));
    }
  }
};

template<typename T>
struct SerializingFrozenVector
{
  std::reference_wrapper<const std::vector<T>> vec;

  explicit SerializingFrozenVector(std::reference_wrapper<const std::vector<T>>&& vec)
      : vec{std::move(vec)}
  {
  }

  template<typename TContext>
  void serialize(const Serializer<TContext>& ser) const
  {
    ser.node |= ryml::SEQ;
    for(auto& element : vec.get())
    {
      const auto tmp = ser.newChild();
      access::dispatch(element, tmp);
    }
  }
};
} // namespace serialization
