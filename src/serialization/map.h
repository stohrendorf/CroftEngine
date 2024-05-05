#pragma once

#include "access.h"

#include <map>
#include <ryml.hpp>
#include <type_traits>

namespace serialization
{
// TODO this works only for very trivial types, but not types that are essentially trivial but are tagged, like IDs or references
template<typename T>
inline constexpr bool isTrivialKey
  = std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_enum_v<T> || std::is_same_v<T, std::string>;

template<typename T, typename U, typename TContext>
void serialize(const std::map<T, U>& data, const Serializer<TContext>& ser)
{
  if constexpr(isTrivialKey<T>)
  {
    ser.node |= ryml::MAP;
    for(auto& [key, value] : data)
    {
      const auto tmp = ser.newChild();
      if constexpr(std::is_enum_v<T>)
      {
        auto enumValue = static_cast<std::underlying_type_t<T>>(key);
        tmp.node << ryml::key(enumValue);
      }
      else
      {
        tmp.node << ryml::key(key);
      }
      access<U, false>::dispatch(value, tmp);
    }
  }
  else
  {
    ser.node |= ryml::SEQ;
    ser.tag("map");
    for(auto& [key, value] : data)
    {
      const auto tmp = ser.newChild();
      access<T, false>::dispatch(key, tmp["key"]);
      access<U, false>::dispatch(value, tmp["value"]);
    }
  }
}

template<typename T, typename U, typename TContext>
void deserialize(std::map<T, U>& data, const Deserializer<TContext>& ser)
{
  if constexpr(isTrivialKey<T>)
  {
    if(ser.node.is_map())
    {
      gsl_Assert(ser.node.is_map());
      data = std::map<T, U>();
      for(const auto& element : ser.node.children())
      {
        gsl_Assert(element.has_key());

        T key{};
        if constexpr(std::is_enum_v<T>)
        {
          std::underlying_type_t<T> enumValue;
          element >> ryml::key(enumValue);
          key = T{enumValue};
        }
        else
        {
          element >> ryml::key(key);
        }
        data.emplace(key, access<U, true>::dispatch(ser.withNode(element)));
      }
      return;
    }
  }

  gsl_Assert(ser.node.is_seq());
  ser.tag("map");
  data = std::map<T, U>();
  for(const auto& element : ser.node.children())
  {
    gsl_Assert(element.is_map());
    gsl_Assert(element.num_children() == 2);
    gsl_Assert(element["key"].valid() && element["value"].valid());

    data.emplace(access<T, true>::dispatch(ser.withNode(element["key"])),
                 access<U, true>::dispatch(ser.withNode(element["value"])));
  }
}
} // namespace serialization
