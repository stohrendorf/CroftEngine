#include "paths.h"

#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <cstdlib>
#include <filesystem>
#include <gsl/gsl-lite.hpp>
#include <sstream>
#include <stdexcept>
#include <string>

std::filesystem::path getExpectedSysUserDataDir()
{
#ifdef WIN32
  const char* appData = getenv("LOCALAPPDATA");
  Expects(appData != nullptr && std::filesystem::is_directory(appData));
  return std::filesystem::path{appData} / "croftengine";
#else
  const char* home = getenv("HOME");
  Expects(home != nullptr && std::filesystem::is_directory(home));
  return std::filesystem::path{home} / ".local" / "share" / "croftengine";
#endif
}

std::filesystem::path getExpectedLocalUserDataDir()
{
  return std::filesystem::current_path();
}

std::filesystem::path getExpectedSysEngineDataDir()
{
#define CE_STRINGIFY2(x) #x
#define CE_STRINGIFY(x) CE_STRINGIFY2(x)
  return std::filesystem::path{CE_STRINGIFY(CE_DATAROOT_DIR)};
#undef CE_STRINGIFY
#undef CE_STRINGIFY2
}

std::filesystem::path getExpectedLocalEngineDataDir()
{
  return std::filesystem::current_path() / "share" / "croftengine";
}

std::optional<std::filesystem::path> findUserDataDir()
{
  static const auto isUserDataDir = [](const std::filesystem::path& path)
  {
    BOOST_LOG_TRIVIAL(info) << "Check user data dir: " << path;
    return std::filesystem::is_directory(path / "data");
  };

  if(auto cwd = getExpectedLocalUserDataDir(); isUserDataDir(cwd))
  {
    BOOST_LOG_TRIVIAL(info) << "User data dir: " << cwd;
    return cwd;
  }

  if(auto cwd = getExpectedSysUserDataDir(); isUserDataDir(cwd))
  {
    BOOST_LOG_TRIVIAL(info) << "User data dir: " << cwd;
    return cwd;
  }

  return std::nullopt;
}

std::optional<std::filesystem::path> findEngineDataDir()
{
  static const auto isEngineDataDir = [](const std::filesystem::path& path)
  {
    BOOST_LOG_TRIVIAL(info) << "Check engine data dir: " << path;
    return std::filesystem::is_regular_file(path / "trfont.ttf");
  };

  if(auto cwd = getExpectedLocalEngineDataDir(); isEngineDataDir(cwd))
  {
    BOOST_LOG_TRIVIAL(info) << "Engine data dir: " << cwd;
    return cwd;
  }

  // when running locally for development
  if(auto cwd = std::filesystem::current_path() / "share"; isEngineDataDir(cwd))
  {
    BOOST_LOG_TRIVIAL(info) << "Engine data dir: " << cwd;
    return cwd;
  }

  if(auto cwd = getExpectedSysEngineDataDir(); isEngineDataDir(cwd))
  {
    BOOST_LOG_TRIVIAL(info) << "Engine data dir: " << cwd;
    return cwd;
  }

  return std::nullopt;
}
