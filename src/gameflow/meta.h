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

  void serialize(const serialization::Serializer<Meta>& ser);
};
} // namespace gameflow