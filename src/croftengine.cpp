#include "engine/engine.h"
#include "engine/player.h"
#include "engine/script/reflection.h"
#include "engine/script/scriptengine.h"
#include "launcher/launcher.h"
#include "paths.h"
#include "stacktrace.h"

#include <boost/exception/diagnostic_information.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
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
void stacktrace_handler(int signum)
{
  std::signal(signum, SIG_DFL);
  std::cerr << "Signal " << signum << " caught";
  stacktrace::logStacktrace();
  std::raise(SIGABRT);
}

void terminateHandler();
const std::terminate_handler oldTerminateHandler = std::set_terminate(&terminateHandler);
void terminateHandler()
{
  BOOST_LOG_TRIVIAL(fatal) << "Abnormal termination";
  stacktrace::logStacktrace();

  if(oldTerminateHandler != nullptr)
    oldTerminateHandler();
}

const gsl::czstring logFormat = "[%TimeStamp% %Severity% %ThreadID%] %Message%";

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
} // namespace

int main(int argc, char** argv)
{
  std::signal(SIGSEGV, &stacktrace_handler);
  std::signal(SIGABRT, &stacktrace_handler);

  boost::log::add_common_attributes();
#ifndef NDEBUG
  initConsole(boost::log::trivial::trace);
#endif

  bool fileLogAdded = false;
  if(const auto userDataDir = findUserDataDir(); userDataDir.has_value())
  {
    boost::log::add_file_log(boost::log::keywords::file_name = (findUserDataDir().value() / "croftengine.log").string(),
                             boost::log::keywords::format = logFormat,
                             boost::log::keywords::auto_flush = true);
    fileLogAdded = true;
  }
  else
  {
#ifdef NDEBUG
    initConsole(boost::log::trivial::info);
#endif
    BOOST_LOG_TRIVIAL(warning) << "Could not determine the user data dir";
  }

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
    boost::log::add_file_log(boost::log::keywords::file_name = (findUserDataDir().value() / "croftengine.log").string(),
                             boost::log::keywords::format = logFormat,
                             boost::log::keywords::auto_flush = true);
  }

  BOOST_LOG_TRIVIAL(info) << "Running CroftEngine " << CE_VERSION;

  try
  {
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
      Expects(meta.has_value());
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
        Expects(!doLoad);
        player = std::make_shared<engine::Player>();
        for(const auto& item : gameflow.getEarlyBoot())
          runResult = engine.runLevelSequenceItem(*item, player, levelStartPlayer);
        break;
      case Mode::Title:
        Expects(!doLoad);
        player = std::make_shared<engine::Player>();
        runResult = engine.runLevelSequenceItem(*gameflow.getTitleMenu(), player, levelStartPlayer);
        break;
      case Mode::Gym:
        Expects(!doLoad);
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
          BOOST_ASSERT(player != nullptr);
          player = std::make_shared<engine::Player>(*levelStartPlayer);
          break;
        }
        break;
      }
    }
  }
  catch(...)
  {
    BOOST_LOG_TRIVIAL(fatal) << boost::current_exception_diagnostic_information();
    stacktrace::logStacktrace();
    return EXIT_FAILURE;
  }
}
