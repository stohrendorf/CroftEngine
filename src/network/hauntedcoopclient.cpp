#include "hauntedcoopclient.h"

// FIXME: this is a bad include path
#include "../launcher/networkconfig.h"

#ifdef WIN32
// workaround to populate the correct windows version to boost asio
#  include <sdkddkver.h>
#endif

#include <algorithm>
#include <atomic>
#include <boost/algorithm/string/split.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/basic_resolver_results.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/socket_base.hpp>
#include <boost/assert.hpp>
#include <boost/cstdint.hpp>
#include <boost/log/trivial.hpp>
#include <exception>
#include <gsl/gsl-lite.hpp>
#include <istream>
#include <iterator>
#include <utility>

namespace
{
enum class ClientMessageId : uint8_t
{
  Login = 0,
  UpdateState = 1,
  StateQuery = 2,
  Failure = 3
};

enum class ServerMessageId : uint8_t
{
  ServerInfo = 0,
  Failure = 1,
  UpdateState = 2,
  FullSync = 3,
};

void writeLogin(std::vector<uint8_t>& msg,
                const std::string& user,
                const std::string& authToken,
                const std::string& sessionId)
{
  msg.emplace_back(static_cast<uint8_t>(ClientMessageId::Login));
  network::io::writePascal(msg, user);
  network::io::writePascal(msg, authToken);
  network::io::writePascal(msg, sessionId);
}

void writeUpdateState(std::vector<uint8_t>& msg, const std::vector<uint8_t>& data)
{
  msg.emplace_back(static_cast<uint8_t>(ClientMessageId::UpdateState));
  network::io::writePascal(msg, data);
}

void write_query_state(std::vector<uint8_t>& msg)
{
  msg.emplace_back(static_cast<uint8_t>(ClientMessageId::StateQuery));
}
} // namespace

namespace network
{
struct HauntedCoopClient::ClientImpl
{
  explicit ClientImpl(std::string gameflowId, std::string levelId)
      : m_resolver{m_ioService}
      , m_socket{m_ioContext}
      , m_gameflowId{std::move(gameflowId)}
      , m_levelId{std::move(levelId)}
  {
    BOOST_LOG_TRIVIAL(info) << "initializing client";

    if(!m_networkConfig.isValid())
    {
      BOOST_LOG_TRIVIAL(info) << "invalid network configuration";
      return;
    }

    if(!m_networkConfig.online)
    {
      BOOST_LOG_TRIVIAL(debug) << "offline mode";
      return;
    }

    gsl_Expects(m_networkConfig.color.size() == 3);

    std::vector<std::string> socketParts;
    boost::algorithm::split(socketParts,
                            m_networkConfig.socket,
                            [](char c)
                            {
                              return c == ':';
                            });
    gsl_Assert(socketParts.size() == 2);

    m_resolver.async_resolve(
      socketParts[0],
      socketParts[1],
      [this](const boost::system::error_code& err, const boost::asio::ip::tcp::resolver::results_type& endpoints)
      {
        if(err)
        {
          BOOST_LOG_TRIVIAL(error) << "resolve failed: " << err.message();
          return;
        }

        BOOST_LOG_TRIVIAL(info) << "service resolved, attempting connect";
        boost::asio::async_connect(
          m_socket,
          endpoints,
          [this](const boost::system::error_code& err, const boost::asio::ip::tcp::endpoint& endpoint)
          {
            BOOST_LOG_TRIVIAL(info) << "connected to " << endpoint.address().to_string() << ":" << endpoint.port();
            onConnected(err);
          });
      });
  }

  void run()
  {
    m_ioService.run();
    m_ioContext.run();
  }

  void close()
  {
    m_ioContext.stop();
    m_ioService.stop();
    if(m_socket.is_open())
    {
      m_socket.shutdown(boost::asio::socket_base::shutdown_type::shutdown_both);
      m_socket.close();
    }
  }

  [[nodiscard]] auto getStates()
  {
    const std::lock_guard guard{m_peerDatasMutex};
    return m_peerDatas;
  }

  void sendState(PeerData data)
  {
    if(!m_loggedIn || !m_socket.is_open())
      return;

    {
      const std::lock_guard guard{m_sendMutex};
      m_sendBuffer.clear();

      data.emplace_back(m_networkConfig.color.at(0));
      data.emplace_back(m_networkConfig.color.at(1));
      data.emplace_back(m_networkConfig.color.at(2));
      io::writePascal(data, m_networkConfig.username);
      writeUpdateState(m_sendBuffer, data);

      ++m_fullSyncCounter;
      if(m_fullSyncCounter >= 30 * 5)
      {
        m_fullSyncCounter = 0;
        write_query_state(m_sendBuffer);
      }

      if(!send())
      {
        BOOST_LOG_TRIVIAL(error) << "send state failed";
        m_loggedIn = false;
        return;
      }
    }
  }

private:
  void onConnected(const boost::system::error_code& err)
  {
    if(err)
    {
      BOOST_LOG_TRIVIAL(error) << "connection failed: " << err.message();
      return;
    }

    try
    {
      // m_socket.non_blocking(true);
      m_socket.set_option(boost::asio::socket_base::keep_alive{true});
      m_socket.set_option(boost::asio::ip::tcp::no_delay{true});
    }
    catch(std::exception& ex)
    {
      BOOST_LOG_TRIVIAL(error) << "error setting socket options: " << ex.what();
      throw;
    }

    {
      BOOST_LOG_TRIVIAL(info) << "Logging in to Haunted Coop Server";
      const std::lock_guard guard{m_sendMutex};
      m_sendBuffer.clear();
      writeLogin(m_sendBuffer,
                 m_networkConfig.username,
                 m_networkConfig.authToken,
                 m_networkConfig.sessionId + "/" + m_gameflowId + "/" + m_levelId);
      if(!send())
      {
        BOOST_LOG_TRIVIAL(error) << "failed to send login credentials";
        m_loggedIn = false;
        return;
      }
    }

    BOOST_LOG_TRIVIAL(info) << "awaiting login response";
    continueWithRead(1, &ClientImpl::awaitLogin);
  }

  void handleServerInfo()
  {
    const auto protocolVersion = io::readLE<uint16_t>(&m_recvBuffer[0]);
    const auto messageSizeLimit = io::readLE<uint16_t>(&m_recvBuffer[2]);
    BOOST_LOG_TRIVIAL(info) << "Connection established. Server protocol " << protocolVersion << ", message size limit "
                            << messageSizeLimit;
    m_loggedIn = true;
    processMessages();
  }

  void awaitLogin()
  {
    switch(static_cast<ServerMessageId>(m_recvBuffer[0]))
    {
    case ServerMessageId::ServerInfo:
      continueWithRead(4, &ClientImpl::handleServerInfo);
      break;

    case ServerMessageId::Failure:
      BOOST_LOG_TRIVIAL(error) << "Login failed";
      continueWithRead(sizeof(uint8_t), &ClientImpl::readFailureLength);
      break;

    default:
      BOOST_LOG_TRIVIAL(error) << "Got unexpected message type " << static_cast<uint16_t>(m_recvBuffer[0]);
      break;
    }
  }

  void readPeerId()
  {
    m_peerId = io::readLE<PeerId>(m_recvBuffer.data());
    continueWithRead(sizeof(uint16_t), &ClientImpl::readPeerDataSize);
  }

  void readPeerDataSize()
  {
    continueWithRead(io::readLE<uint16_t>(m_recvBuffer.data()), &ClientImpl::readPeerData);
  }

  void readPeerData()
  {
    {
      const std::lock_guard guard{m_peerDatasMutex};
      m_peerDatas.insert_or_assign(m_peerId, m_recvBuffer);
    }
    processMessages();
  }

  void readFailureLength()
  {
    continueWithRead(m_recvBuffer[0], &ClientImpl::readFailure);
  }

  void readFailure()
  {
    std::string tmp{m_recvBuffer.begin(), m_recvBuffer.end()};
    BOOST_LOG_TRIVIAL(error) << "received failure message from server: " << tmp;
    processMessages();
  }

  void readFullSyncStatesCount()
  {
    m_fullSyncStatesCount = io::readLE<uint8_t>(m_recvBuffer.data());
    m_fullSyncPeerDatas.clear();

    if(m_fullSyncStatesCount > 0)
      continueWithRead(sizeof(PeerId) + sizeof(uint16_t), &ClientImpl::readFullSyncPeerIdAndDataSize);
    else
    {
      const std::lock_guard guard{m_peerDatasMutex};
      m_peerDatas.clear();
      processMessages();
    }
  }

  void readFullSyncPeerIdAndDataSize()
  {
    m_peerId = io::readLE<PeerId>(m_recvBuffer.data());
    continueWithRead(io::readLE<uint16_t>(&m_recvBuffer[sizeof(PeerId)]), &ClientImpl::readFullSyncPeerData);
  }

  void readFullSyncPeerData()
  {
    m_fullSyncPeerDatas.insert_or_assign(m_peerId, m_recvBuffer);
    BOOST_ASSERT(m_fullSyncStatesCount > 0);
    --m_fullSyncStatesCount;
    if(m_fullSyncStatesCount > 0)
      continueWithRead(sizeof(PeerId) + sizeof(uint16_t), &ClientImpl::readFullSyncPeerIdAndDataSize);
    else
    {
      const std::lock_guard guard{m_peerDatasMutex};
      m_peerDatas = std::move(m_fullSyncPeerDatas);
      processMessages();
    }
  }

  void dispatchMessage()
  {
    switch(static_cast<ServerMessageId>(m_recvBuffer[0]))
    {
    case ServerMessageId::UpdateState:
      continueWithRead(sizeof(PeerId), &ClientImpl::readPeerId);
      break;

    case ServerMessageId::ServerInfo:
      BOOST_LOG_TRIVIAL(error) << "received unexpected server info";
      break;

    case ServerMessageId::Failure:
      continueWithRead(sizeof(uint8_t), &ClientImpl::readFailureLength);
      break;

    case ServerMessageId::FullSync:
      continueWithRead(sizeof(uint8_t), &ClientImpl::readFullSyncStatesCount);
      break;

    default:
      BOOST_LOG_TRIVIAL(error) << "received unexpected message id " << static_cast<uint16_t>(m_recvBuffer[0]);
      break;
    }
  }

  void processMessages()
  {
    continueWithRead(1, &ClientImpl::dispatchMessage);
  }

  bool send()
  {
    try
    {
      m_socket.send(boost::asio::buffer(m_sendBuffer));
    }
    catch(std::exception& ex)
    {
      BOOST_LOG_TRIVIAL(error) << "Failed to send data: " << ex.what();
      m_socket.close();
      return false;
    }

    return true;
  }

  boost::asio::io_service m_ioService;
  boost::asio::ip::tcp::resolver m_resolver;
  boost::asio::io_context m_ioContext;
  boost::asio::ip::tcp::socket m_socket;

  std::atomic_bool m_loggedIn{false};

  std::mutex m_sendMutex;
  std::vector<uint8_t> m_sendBuffer;
  std::vector<uint8_t> m_recvBuffer;

  PeerId m_peerId = 0;

  uint16_t m_fullSyncStatesCount = 0;
  std::map<PeerId, PeerData> m_fullSyncPeerDatas{};

  std::mutex m_peerDatasMutex;
  std::map<PeerId, PeerData> m_peerDatas{};

  uint16_t m_fullSyncCounter = 0;

  launcher::NetworkConfig m_networkConfig{launcher::NetworkConfig::load()};
  std::string m_gameflowId;
  std::string m_levelId;

  using ContinueWithReadFn = void (ClientImpl::*)();
  void continueWithRead(size_t n, ContinueWithReadFn fn)
  {
    if(!m_socket.is_open())
      return;

    m_recvBuffer.resize(n);
    boost::asio::async_read(m_socket,
                            boost::asio::buffer(m_recvBuffer),
                            [this, fn](const boost::system::error_code& err, std::size_t bytesTransferred)
                            {
                              if(err)
                              {
                                BOOST_LOG_TRIVIAL(error) << "read failed: " << err.message();
                                return;
                              }

                              if(bytesTransferred != m_recvBuffer.size())
                              {
                                BOOST_LOG_TRIVIAL(error) << "read failed: wanted " << m_recvBuffer.size()
                                                         << " bytes, got " << bytesTransferred;
                                return;
                              }

                              (this->*fn)();
                            });
  }
};

HauntedCoopClient::HauntedCoopClient(const std::string& gameflowId, const std::string& levelId)
    : impl{std::make_unique<ClientImpl>(gameflowId, levelId)}
{
}

void HauntedCoopClient::sendState(const PeerData& data)
{
  impl->sendState(data);
}

void HauntedCoopClient::updateThread()
{
  impl->run();
}

std::map<PeerId, PeerData> HauntedCoopClient::getStates() const
{
  const std::lock_guard guard{m_statesMutex};
  return impl->getStates();
}

void HauntedCoopClient::start()
{
  m_thread = std::thread{&HauntedCoopClient::updateThread, this};
}

HauntedCoopClient::~HauntedCoopClient()
{
  stop();
}

void HauntedCoopClient::stop()
{
  impl->close();
  if(m_thread.joinable())
    m_thread.join();
}

void io::writePascal(std::vector<uint8_t>& msg, const std::string& s)
{
  msg.reserve(msg.size() + s.length());
  msg.emplace_back(static_cast<uint8_t>(s.length()));
  std::copy(s.begin(), s.end(), std::back_inserter(msg));
}

void io::writePascal(std::vector<uint8_t>& msg, const std::vector<uint8_t>& s)
{
  writeLE<uint16_t>(msg, gsl::narrow<uint16_t>(s.size()));
  msg.reserve(msg.size() + s.size());
  std::copy(s.begin(), s.end(), std::back_inserter(msg));
}

std::string io::readPascalString(const uint8_t* msg)
{
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  return std::string{reinterpret_cast<const char*>(&msg[1]), msg[0]};
}

std::string io::readPascalString(std::istream& stream)
{
  uint8_t length = 0;
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  stream.read(reinterpret_cast<char*>(&length), sizeof(length));
  gsl_Assert(stream.gcount() == sizeof(length));
  std::vector<char> buffer;
  buffer.resize(length);
  stream.read(buffer.data(), gsl::narrow<std::streamsize>(buffer.size()));
  gsl_Assert(gsl::narrow<size_t>(stream.gcount()) == buffer.size());
  return std::string{buffer.data(), buffer.size()};
}
} // namespace network
