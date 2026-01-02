#pragma once

#include "access.h"
#include "serialization.h" // IWYU pragma: keep

#include <gsl-lite/gsl-lite.hpp>
#include <map>


namespace serialization
{
template<typename T, typename U, typename TContext>
void serialize(const std::map<T, U>& data, const Serializer<TContext>& ser)
{
  ser.node |= ryml::SEQ;
  ser.tag("map");
  for(auto& [key, value] : data)
  {
    const auto tmp = ser.newChild();
    access::dispatch(key, tmp["key"]);
    access::dispatch(value, tmp["value"]);
  }
}

template<typename T, typename U, typename TContext>
void deserialize(std::map<T, U>& data, const Deserializer<TContext>& ser)
{
  ser.tag("map");
  data = std::map<T, U>();
  for(const auto& element : ser.node.children())
  {
    gsl_Assert(element.is_map());
    gsl_Assert(element.num_children() == 2);
    gsl_Assert(element["key"].readable() && element["value"].readable());

    data.emplace(access::dispatch<T>(ser.withNode(element["key"])),
                 access::dispatch<U>(ser.withNode(element["value"])));
  }
}
} // namespace serialization