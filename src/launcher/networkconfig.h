#pragma once

// FIXME: this is a bad include path
#include "../serialization/serialization_fwd.h"

#include <cstdint>
#include <string>
#include <vector>

namespace launcher
{
struct NetworkConfig
{
  std::string socket;
  std::string username;
  std::string authToken;
  std::string sessionId;
  std::vector<uint8_t> color;

  void serialize(const serialization::Serializer<NetworkConfig>& ser) const;
  void deserialize(const serialization::Deserializer<NetworkConfig>& ser);

  static NetworkConfig load();

  void save();

  [[nodiscard]] bool isValid() const;
};
} // namespace launcher
