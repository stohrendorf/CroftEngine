#include "engine/engine.h"
#include "engine/player.h"
#include "engine/script/reflection.h"
#include "engine/script/scriptengine.h"
#include "launcher/launcher.h"
#include "paths.h"

#include <boost/exception/diagnostic_information.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <chillout.h>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <gsl/gsl-lite.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#ifdef WIN32
#  include <windows.h>
#endif

namespace
{

const gsl::czstring logFormat = "[%TimeStamp% %Severity% %ThreadID%] %Message%";

using sink_t = boost::log::sinks::synchronous_sink<boost::log::sinks::text_file_backend>;
boost::shared_ptr<sink_t> logFileSink;

void initConsole(boost::log::trivial::severity_level level)
{
#ifdef WIN32
  AllocConsole();

  // https://stackoverflow.com/a/57241985
  FILE* fDummy;
  freopen_s(&fDummy, "CONIN$", "r", stdin);
  freopen_s(&fDummy, "CONOUT$", "w", stderr);
  freopen_s(&fDummy, "CONOUT$", "w", stdout);
#endif
  boost::log::add_console_log(std::cout, boost::log::keywords::format = logFormat)
    ->set_filter(boost::log::trivial::severity >= level);
}

void initFileLogging(const std::filesystem::path& userDataDir)
{
  logFileSink = boost::log::add_file_log(boost::log::keywords::target = userDataDir,
                                         boost::log::keywords::file_name = userDataDir / "croftengine.%N.log",
                                         boost::log::keywords::format = logFormat,
                                         boost::log::keywords::auto_flush = true,
                                         boost::log::keywords::max_files = 10);
}

bool initCrashReporting()
{
  auto& chillout = Debug::Chillout::getInstance();
  const auto userDataDir = findUserDataDir();
  if(!userDataDir.has_value())
  {
    return false;
  }

  auto crashdumpDir = userDataDir.value() / "crashdumps";
  if(!std::filesystem::is_directory(crashdumpDir))
  {
    std::filesystem::create_directory(crashdumpDir);
  }
#ifdef WIN32
  chillout.init(L"croftengine", crashdumpDir);
#else
  chillout.init("croftengine", crashdumpDir);
#endif
  chillout.setBacktraceCallback(
    [](const char* stackTraceEntry)
    {
      BOOST_LOG_TRIVIAL(fatal) << stackTraceEntry;
    });
  chillout.setCrashCallback(
    [&chillout]()
    {
      BOOST_LOG_TRIVIAL(fatal) << "Croft engine has crashed, writing minidump";
      chillout.backtrace();
#ifdef WIN32
      chillout.createCrashDump();
#endif
      if(logFileSink != nullptr)
      {
        // https://stackoverflow.com/questions/41419957/boostlog-close-log-file-and-open-a-new-one/41420226#41420226
        logFileSink->locked_backend()->rotate_file();
      }
    });
  return true;
}

} // namespace

int main(int argc, char** argv)
{
#ifdef NDEBUG
  bool crashReportInitSuccess = initCrashReporting();
#endif
  boost::log::add_common_attributes();
#ifndef NDEBUG
  initConsole(boost::log::trivial::trace);
#endif

  bool fileLogAdded = false;
  if(const auto userDataDir = findUserDataDir(); userDataDir.has_value())
  {
    initFileLogging(*userDataDir);
    fileLogAdded = true;
  }
  else
  {
#ifdef NDEBUG
    initConsole(boost::log::trivial::info);
#endif
    BOOST_LOG_TRIVIAL(warning) << "Could not determine the user data dir";
  }

#ifdef NDEBUG
  if(!crashReportInitSuccess)
  {
    BOOST_LOG_TRIVIAL(warning) << "Crash report initialization failed (nowhere to write dumps)";
  }
#endif

  std::string localeOverride;
  std::string gameflowId;
  {
    const auto launcherResult = launcher::showLauncher(argc, argv);
    if(!launcherResult.has_value())
    {
      return EXIT_SUCCESS;
    }

    localeOverride = std::get<0>(*launcherResult);
    gameflowId = std::get<1>(*launcherResult);
  }

  if(!fileLogAdded)
  {
    initFileLogging(findUserDataDir().value());
  }

  BOOST_LOG_TRIVIAL(info) << "Running CroftEngine " << CE_VERSION;

  engine::Engine engine{findUserDataDir().value(), findEngineDataDir().value(), localeOverride, gameflowId};
  size_t levelSequenceIndex = 0;
  enum class Mode
  {
    Boot,
    Title,
    Gym,
    Game
  };
  auto mode = Mode::Boot;
  std::optional<size_t> loadSlot;
  bool doLoad = false;

  const auto& gameflow = engine.getScriptEngine().getGameflow();
  auto processLoadRequest
    = [&engine, &levelSequenceIndex, &mode, &loadSlot, &doLoad, &gameflow](const std::optional<size_t>& slot) -> void
  {
    const auto meta = engine.getSavegameMeta(slot);
    gsl_Assert(meta.has_value());
    for(levelSequenceIndex = 0; levelSequenceIndex < gameflow.getLevelSequence().size(); ++levelSequenceIndex)
    {
      if(gameflow.getLevelSequence().at(levelSequenceIndex)->isLevel(meta->filename))
        break;
    }
    loadSlot = slot;
    doLoad = true;
    mode = Mode::Game;
  };

  std::shared_ptr<engine::Player> player;
  std::shared_ptr<engine::Player> levelStartPlayer;

  while(true)
  {
    std::pair<engine::RunResult, std::optional<size_t>> runResult;
    switch(mode)
    {
    case Mode::Boot:
      gsl_Assert(!doLoad);
      player = std::make_shared<engine::Player>();
      for(const auto& item : gameflow.getEarlyBoot())
        runResult = engine.runLevelSequenceItem(*item, player, levelStartPlayer);
      break;
    case Mode::Title:
      gsl_Assert(!doLoad);
      player = std::make_shared<engine::Player>();
      runResult = engine.runLevelSequenceItem(*gameflow.getTitleMenu(), player, levelStartPlayer);
      break;
    case Mode::Gym:
      gsl_Assert(!doLoad);
      player = std::make_shared<engine::Player>();
      for(const auto& item : gameflow.getLaraHome())
        runResult = engine.runLevelSequenceItem(*item, player, levelStartPlayer);
      break;
    case Mode::Game:
      if(doLoad)
      {
        player = std::make_shared<engine::Player>();
        levelStartPlayer = std::make_shared<engine::Player>();
        runResult = engine.runLevelSequenceItemFromSave(
          *gsl::not_null{gameflow.getLevelSequence().at(levelSequenceIndex)}, loadSlot, player, levelStartPlayer);
      }
      else
      {
        if(player == nullptr || levelSequenceIndex == 0)
        {
          if(levelStartPlayer == nullptr)
            player = std::make_shared<engine::Player>();
          else
            player = std::make_shared<engine::Player>(*levelStartPlayer);
        }
        levelStartPlayer = std::make_shared<engine::Player>(*player);

        runResult = engine.runLevelSequenceItem(
          *gsl::not_null{gameflow.getLevelSequence().at(levelSequenceIndex)}, player, levelStartPlayer);
      }
      break;
    }

    loadSlot.reset();
    doLoad = false;

    switch(mode)
    {
    case Mode::Boot:
      if(runResult.first == engine::RunResult::ExitApp)
        return EXIT_SUCCESS;
      mode = Mode::Title;
      break;
    case Mode::Title:
      switch(runResult.first)
      {
      case engine::RunResult::ExitApp:
        return EXIT_SUCCESS;
      case engine::RunResult::NextLevel:
        levelSequenceIndex = 0;
        mode = Mode::Game;
        levelStartPlayer.reset();
        break;
      case engine::RunResult::TitleLevel:
        mode = Mode::Title;
        break;
      case engine::RunResult::LaraHomeLevel:
        mode = Mode::Gym;
        break;
      case engine::RunResult::RequestLoad:
        processLoadRequest(runResult.second);
        break;
      case engine::RunResult::RestartLevel:
        BOOST_THROW_EXCEPTION(std::runtime_error("level restart request while running title menu"));
      }
      break;
    case Mode::Gym:
      switch(runResult.first)
      {
      case engine::RunResult::ExitApp:
        return EXIT_SUCCESS;
      case engine::RunResult::NextLevel:
        mode = Mode::Title;
        break;
      case engine::RunResult::TitleLevel:
        mode = Mode::Title;
        break;
      case engine::RunResult::LaraHomeLevel:
        mode = Mode::Gym;
        break;
      case engine::RunResult::RequestLoad:
        processLoadRequest(runResult.second);
        break;
      case engine::RunResult::RestartLevel:
        BOOST_THROW_EXCEPTION(std::runtime_error("level restart request while running title menu"));
      }
      break;
    case Mode::Game:
      switch(runResult.first)
      {
      case engine::RunResult::ExitApp:
        return EXIT_SUCCESS;
      case engine::RunResult::NextLevel:
        ++levelSequenceIndex;
        if(levelSequenceIndex >= gameflow.getLevelSequence().size())
        {
          levelSequenceIndex = 0;
          mode = Mode::Title;
        }
        else
        {
          BOOST_ASSERT(player != nullptr);
          player->accumulateStats();
        }
        break;
      case engine::RunResult::TitleLevel:
        mode = Mode::Title;
        break;
      case engine::RunResult::LaraHomeLevel:
        mode = Mode::Gym;
        break;
      case engine::RunResult::RequestLoad:
        processLoadRequest(runResult.second);
        break;
      case engine::RunResult::RestartLevel:
        BOOST_ASSERT(levelStartPlayer != nullptr);
        player = std::make_shared<engine::Player>(*levelStartPlayer);
        break;
      }
      break;
    }
  }
}
