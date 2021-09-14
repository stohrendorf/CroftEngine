#include "engine/engine.h"
#include "engine/player.h"
#include "engine/script/reflection.h"
#include "engine/script/scriptengine.h"

#include <boost/exception/diagnostic_information.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/stacktrace/stacktrace.hpp>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <gsl/gsl-lite.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <vector>

namespace
{
void stacktrace_handler(int signum)
{
  std::signal(signum, SIG_DFL);
  std::cerr << "Signal " << signum << " caught; stacktrace:\n" << boost::stacktrace::stacktrace();
  std::raise(SIGABRT);
}

void terminateHandler();
const std::terminate_handler oldTerminateHandler = std::set_terminate(&terminateHandler);
void terminateHandler()
{
  BOOST_LOG_TRIVIAL(error) << "Abnormal termination. Stacktrace:\n" << boost::stacktrace::stacktrace();

  if(oldTerminateHandler != nullptr)
    oldTerminateHandler();
}
} // namespace

int main()
{
  std::signal(SIGSEGV, &stacktrace_handler);
  std::signal(SIGABRT, &stacktrace_handler);

  static const gsl::czstring logFormat = "[%TimeStamp% %Severity% %ThreadID%] %Message%";

#ifndef NDEBUG
  static const auto consoleMinSeverity = boost::log::trivial::trace;
#else
  static const auto consoleMinSeverity = boost::log::trivial::info;
#endif

  boost::log::add_common_attributes();
  boost::log::add_console_log(std::cout, boost::log::keywords::format = logFormat)
    ->set_filter(boost::log::trivial::severity >= consoleMinSeverity);
  boost::log::add_file_log(boost::log::keywords::file_name = "edisonengine.log",
                           boost::log::keywords::format = logFormat,
                           boost::log::keywords::auto_flush = true);

  engine::Engine engine{std::filesystem::current_path()};
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

  auto processLoadRequest
    = [&engine, &levelSequenceIndex, &mode, &loadSlot, &doLoad](const std::optional<size_t>& slot) -> void
  {
    const auto meta = engine.getSavegameMeta(slot);
    Expects(meta.has_value());
    for(levelSequenceIndex = 0; true; ++levelSequenceIndex)
    {
      auto item = engine.getScriptEngine().getLevelSequenceItem(levelSequenceIndex);
      if(item == nullptr || item->isLevel(meta->filename))
        break;
    }
    Expects(engine.getScriptEngine().getLevelSequenceItem(levelSequenceIndex) != nullptr);
    loadSlot = slot;
    doLoad = true;
    mode = Mode::Game;
  };

  std::shared_ptr<engine::Player> player;

  while(true)
  {
    std::pair<engine::RunResult, std::optional<size_t>> runResult;
    switch(mode)
    {
    case Mode::Boot:
      Expects(!doLoad);
      player = std::make_shared<engine::Player>();
      for(const auto& item : engine.getScriptEngine().getEarlyBoot())
        runResult = engine.runLevelSequenceItem(*item, player);
      break;
    case Mode::Title:
      Expects(!doLoad);
      player = std::make_shared<engine::Player>();
      runResult = engine.runLevelSequenceItem(*engine.getScriptEngine().getTitleMenu(), player);
      break;
    case Mode::Gym:
      Expects(!doLoad);
      player = std::make_shared<engine::Player>();
      for(const auto& item : engine.getScriptEngine().getLaraHome())
        runResult = engine.runLevelSequenceItem(*item, player);
      break;
    case Mode::Game:
      if(doLoad)
      {
        player = std::make_shared<engine::Player>();
        runResult = engine.runLevelSequenceItemFromSave(
          *gsl::not_null{engine.getScriptEngine().getLevelSequenceItem(levelSequenceIndex)}, loadSlot, player);
      }
      else
      {
        if(player == nullptr || levelSequenceIndex == 0)
          player = std::make_shared<engine::Player>();

        runResult = engine.runLevelSequenceItem(
          *gsl::not_null{engine.getScriptEngine().getLevelSequenceItem(levelSequenceIndex)}, player);
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
      case engine::RunResult::ExitApp: return EXIT_SUCCESS;
      case engine::RunResult::NextLevel:
        levelSequenceIndex = 0;
        mode = Mode::Game;
        break;
      case engine::RunResult::TitleLevel: mode = Mode::Title; break;
      case engine::RunResult::LaraHomeLevel: mode = Mode::Gym; break;
      case engine::RunResult::RequestLoad: processLoadRequest(runResult.second); break;
      }
      break;
    case Mode::Gym:
      switch(runResult.first)
      {
      case engine::RunResult::ExitApp: return EXIT_SUCCESS;
      case engine::RunResult::NextLevel: mode = Mode::Title; break;
      case engine::RunResult::TitleLevel: mode = Mode::Title; break;
      case engine::RunResult::LaraHomeLevel: mode = Mode::Gym; break;
      case engine::RunResult::RequestLoad: processLoadRequest(runResult.second); break;
      }
      break;
    case Mode::Game:
      switch(runResult.first)
      {
      case engine::RunResult::ExitApp: return EXIT_SUCCESS;
      case engine::RunResult::NextLevel:
        ++levelSequenceIndex;
        if(engine.getScriptEngine().getLevelSequenceItem(levelSequenceIndex) == nullptr)
        {
          levelSequenceIndex = 0;
          mode = Mode::Title;
        }
        break;
      case engine::RunResult::TitleLevel: mode = Mode::Title; break;
      case engine::RunResult::LaraHomeLevel: mode = Mode::Gym; break;
      case engine::RunResult::RequestLoad: processLoadRequest(runResult.second); break;
      }
      break;
    }
  }
}
