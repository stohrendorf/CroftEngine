#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <filesystem>
#include <gsl/gsl-lite.hpp>
#include <stdexcept>

std::filesystem::path getSysUserDataDir()
{
#ifdef WIN32
  const char* appData = getenv("LOCALAPPDATA");
  Expects(appData != nullptr && std::filesystem::is_directory(appData));
  return std::filesystem::path{appData} / "edisonengine";
#else
  const char* home = getenv("HOME");
  Expects(home != nullptr && std::filesystem::is_directory(home));
  return std::filesystem::path{home} / ".local" / "share" / "edisonengine";
#endif
}

#define EE_STRINGIFY2(x) #x
#define EE_STRINGIFY(x) EE_STRINGIFY2(x)

std::filesystem::path getSysEngineDataDir()
{
  return std::filesystem::path{EE_STRINGIFY(EE_DATAROOT_DIR)};
}

std::filesystem::path getUserDataDir()
{
  static const auto isUserDataDir = [](const std::filesystem::path& path)
  {
    BOOST_LOG_TRIVIAL(info) << "Check user data dir: " << path;
    return std::filesystem::is_regular_file(path / "data" / "tr1" / "DATA" / "TITLEH.PCX");
  };

  if(auto cwd = std::filesystem::current_path(); isUserDataDir(cwd))
  {
    BOOST_LOG_TRIVIAL(info) << "User data dir: " << cwd;
    return cwd;
  }

  if(auto cwd = getSysUserDataDir(); isUserDataDir(cwd))
  {
    BOOST_LOG_TRIVIAL(info) << "User data dir: " << cwd;
    return cwd;
  }

  BOOST_THROW_EXCEPTION(std::runtime_error("Failed to determine user data dir"));
}

std::filesystem::path getEngineDataDir()
{
  static const auto isEngineDataDir = [](const std::filesystem::path& path)
  {
    BOOST_LOG_TRIVIAL(info) << "Check engine data dir: " << path;
    return std::filesystem::is_regular_file(path / "trfont.ttf");
  };

  if(auto cwd = std::filesystem::current_path() / "share" / "edisonengine"; isEngineDataDir(cwd))
  {
    BOOST_LOG_TRIVIAL(info) << "Engine data dir: " << cwd;
    return cwd;
  }

  if(auto cwd = getSysEngineDataDir(); isEngineDataDir(cwd))
  {
    BOOST_LOG_TRIVIAL(info) << "Engine data dir: " << cwd;
    return cwd;
  }

  BOOST_THROW_EXCEPTION(std::runtime_error("Failed to determine engine data dir"));
}
