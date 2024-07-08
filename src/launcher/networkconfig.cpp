#include "networkconfig.h"

#include "paths.h"
#include "serialization/optional_value.h"
#include "serialization/serialization.h"
#include "serialization/vector.h"
#include "serialization/yamldocument.h"

#include <algorithm>
#include <functional>
#include <gsl/gsl-lite.hpp>
#include <regex>

namespace launcher
{
void NetworkConfig::serialize(const serialization::Serializer<NetworkConfig>& ser) const
{
  ser(S_NV("socket", socket),
      S_NV("username", username),
      S_NV("authToken", authToken),
      S_NV("sessionId", sessionId),
      S_NV("color", color),
      S_NV("online", online));
}

void NetworkConfig::deserialize(const serialization::Deserializer<NetworkConfig>& ser)
{
  ser(S_NV("socket", socket),
      S_NV("username", username),
      S_NV("authToken", authToken),
      S_NV("sessionId", sessionId),
      S_NV("color", color),
      S_NVO("online", std::ref(online)));
}

NetworkConfig NetworkConfig::load()
{
  serialization::YAMLDocument<true> doc{findUserDataDir().value() / "network.yaml"};
  NetworkConfig cfg{};
  doc.deserialize("config", gsl::not_null{&cfg}, cfg);
  return cfg;
}

void NetworkConfig::save()
{
  serialization::YAMLDocument<false> doc{findUserDataDir().value() / "network.yaml"};
  doc.serialize("config", gsl::not_null{this}, *this);
  doc.write();
}

bool NetworkConfig::isValid() const
{
  if(std::count(socket.begin(), socket.end(), ':') != 1)
    return false;
  if(username.empty())
    return false;

  static const std::regex tokenRegex{"[a-f0-9]{32}"};
  if(!std::regex_match(authToken, tokenRegex))
    return false;
  if(!std::regex_match(sessionId, tokenRegex))
    return false;

  return true;
}
} // namespace launcher
