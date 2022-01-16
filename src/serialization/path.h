#pragma once

#include "access.h"

#include <filesystem>
#include <ryml.hpp>

namespace serialization
{
template<typename TContext>
void save(std::filesystem::path& data, const Serializer<TContext>& ser)
{
  ser.tag("path");
  auto tmp = data.string();
  access<std::string>::callSerializeOrSave(tmp, ser);
}

template<typename TContext>
void load(std::filesystem::path& data, const Serializer<TContext>& ser)
{
  ser.tag("path");
  std::string tmp;
  access<std::string>::callSerializeOrLoad(tmp, ser);
  data = std::filesystem::path{tmp};
}

template<typename TContext>
std::filesystem::path create(const TypeId<std::filesystem::path>&, const Serializer<TContext>& ser)
{
  std::filesystem::path tmp;
  load(tmp, ser);
  return tmp;
}
} // namespace serialization
