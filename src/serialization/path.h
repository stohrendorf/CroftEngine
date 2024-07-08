#pragma once

#include "access.h" // IWYU pragma: keep
#include "serialization.h"

#include <filesystem>

namespace serialization
{
template<typename TContext>
void serialize(const std::filesystem::path& data, const Serializer<TContext>& ser)
{
  ser.tag("path");
  auto tmp = data.string();
  access<std::string, false>::dispatch(tmp, ser);
}

template<typename TContext>
void deserialize(std::filesystem::path& data, const Deserializer<TContext>& ser)
{
  ser.tag("path");
  std::string tmp;
  access<std::string, true>::dispatch(tmp, ser);
  data = std::filesystem::path{tmp};
}

template<typename TContext>
std::filesystem::path create(const TypeId<std::filesystem::path>&, const Deserializer<TContext>& ser)
{
  std::filesystem::path tmp;
  deserialize(tmp, ser);
  return tmp;
}
} // namespace serialization
