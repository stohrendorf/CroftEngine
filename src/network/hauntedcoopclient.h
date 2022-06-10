#pragma once

#include <atomic>
#include <cstdint>
#include <gsl/gsl-lite.hpp>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

namespace network
{
using PeerId = uint64_t;
using PeerData = std::vector<uint8_t>;

class HauntedCoopClient
{
public:
  explicit HauntedCoopClient(const std::string& gameflowId, const std::string& levelId);
  ~HauntedCoopClient();

  void sendState(const PeerData& data);

  [[nodiscard]] std::map<PeerId, PeerData> getStates() const;

  void start();

  void stop();

private:
  struct ClientImpl;

  mutable std::mutex m_statesMutex;
  std::unique_ptr<ClientImpl> impl;
  std::thread m_thread;

  void updateThread();
};
} // namespace network

namespace network::io
{
template<typename T>
inline void writeLE(std::vector<uint8_t>& msg, const T& data)
{
  for(size_t i = 0; i < sizeof(T); ++i)
  {
    msg.emplace_back(static_cast<uint8_t>(data >> (8u * i)));
  }
}

template<typename T>
inline T readLE(const uint8_t* data)
{
  T result{0};
  for(size_t i = 0; i < sizeof(T); ++i)
  {
    result |= static_cast<T>(data[i]) << (8u * i);
  }
  return result;
}

void writePascal(std::vector<uint8_t>& msg, const std::string& s);
void writePascal(std::vector<uint8_t>& msg, const std::vector<uint8_t>& s);

std::string readPascalString(const uint8_t* msg);
std::string readPascalString(std::istream& stream);
} // namespace network::io
