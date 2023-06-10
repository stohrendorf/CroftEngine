#pragma once

#include "serialization/serialization_fwd.h"

#include <string>
#include <vector>

namespace gameflow
{
struct Meta
{
  std::string title;
  std::string author;
  std::vector<std::string> urls;
  bool downloadSoundtrack = false;

  void serialize(const serialization::Serializer<Meta>& ser) const;
  void deserialize(const serialization::Deserializer<Meta>& ser);
};
} // namespace gameflow