#include "engine/engine.h"
#include "engine/player.h"
#include "engine/script/reflection.h"
#include "engine/script/scriptengine.h"
#include "paths.h"
#include "setup/setup.h"
#include "stacktrace.h"

#include <boost/exception/diagnostic_information.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/program_options.hpp>
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
} // namespace

int main(int argc, char** argv)
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
#ifndef NDEBUG
  boost::log::add_console_log(std::cout, boost::log::keywords::format = logFormat)
    ->set_filter(boost::log::trivial::severity >= consoleMinSeverity);
#endif
  std::optional<std::string> localeOverride;
  {
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()("locale,l",
                       boost::program_options::value<std::string>()->default_value(""),
                       "set locale override, e.g. de_DE.utf8");
    desc.add_options()("configure,c", boost::program_options::bool_switch(), "start configuration ui");

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    if(auto it = vm.find("locale"); it != vm.end() && !it->second.as<std::string>().empty())
    {
      localeOverride = it->second.as<std::string>();
    }

    if(auto it = vm.find("configure"); (it != vm.end() && it->second.as<bool>()) || !findUserDataDir().has_value()
                                       || !std::filesystem::is_directory(*findUserDataDir() / "data" / "tr1"))
    {
      setup::showSetupScreen(argc, argv);
      return EXIT_SUCCESS;
    }
  }

  boost::log::add_file_log(boost::log::keywords::file_name = (findUserDataDir().value() / "edisonengine.log").string(),
                           boost::log::keywords::format = logFormat,
                           boost::log::keywords::auto_flush = true);

  BOOST_LOG_TRIVIAL(info) << "Running EdisonEngine " << EE_VERSION;

  try
  {
    engine::Engine engine{findUserDataDir().value(), findEngineDataDir().value(), localeOverride};
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
      for(levelSequenceIndex = 0; true; ++levelSequenceIndex)
      {
        if(levelSequenceIndex >= gameflow.getLevelSequence().size()
           || gameflow.getLevelSequence().at(levelSequenceIndex)->isLevel(meta->filename))
          break;
      }
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
        for(const auto& item : gameflow.getEarlyBoot())
          runResult = engine.runLevelSequenceItem(*item, player);
        break;
      case Mode::Title:
        Expects(!doLoad);
        player = std::make_shared<engine::Player>();
        runResult = engine.runLevelSequenceItem(*gameflow.getTitleMenu(), player);
        break;
      case Mode::Gym:
        Expects(!doLoad);
        player = std::make_shared<engine::Player>();
        for(const auto& item : gameflow.getLaraHome())
          runResult = engine.runLevelSequenceItem(*item, player);
        break;
      case Mode::Game:
        if(doLoad)
        {
          player = std::make_shared<engine::Player>();
          runResult = engine.runLevelSequenceItemFromSave(
            *gsl::not_null{gameflow.getLevelSequence().at(levelSequenceIndex)}, loadSlot, player);
        }
        else
        {
          if(player == nullptr || levelSequenceIndex == 0)
            player = std::make_shared<engine::Player>();

          runResult
            = engine.runLevelSequenceItem(*gsl::not_null{gameflow.getLevelSequence().at(levelSequenceIndex)}, player);
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
