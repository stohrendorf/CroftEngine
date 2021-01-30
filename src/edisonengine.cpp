#include "engine/engine.h"
#include "engine/script/reflection.h"

#include <boost/exception/diagnostic_information.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/stacktrace.hpp>
#include <csignal>
#include <iostream>

[[maybe_unused]] void gsl::fail_fast_assert_handler(char const* const expression,
                                                    char const* const message,
                                                    char const* const file,
                                                    int line)
{
  BOOST_LOG_TRIVIAL(error) << "Expectation failed at " << file << ":" << line;
  BOOST_LOG_TRIVIAL(error) << "  - expression " << expression;
  BOOST_LOG_TRIVIAL(error) << "  - message " << message;
  BOOST_LOG_TRIVIAL(error) << "Stacktrace:\n" << boost::stacktrace::stacktrace();
  BOOST_THROW_EXCEPTION(gsl::fail_fast(message));
}

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

#ifdef NDEBUG
  boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
#endif

  engine::Engine engine{std::filesystem::current_path()};
  size_t levelSequenceIndex = 0;
  const size_t levelSequenceLength = pybind11::len(pybind11::globals()["level_sequence"]);
  enum class Mode
  {
    Title,
    Gym,
    Game
  };
  auto mode = Mode::Title;
  std::optional<size_t> loadSlot;
  bool doLoad = false;

  auto processLoadRequest = [&engine, &levelSequenceIndex, &levelSequenceLength, &mode, &loadSlot, &doLoad](
                              const std::optional<size_t>& slot) -> void {
    std::string filename;
    if(!slot.has_value())
    {
      filename = engine.getSavegameMeta("quicksave.yaml").filename;
    }
    else
    {
      filename = engine.getSavegameMeta(slot.value()).filename;
    }
    for(levelSequenceIndex = 0; levelSequenceIndex < levelSequenceLength; ++levelSequenceIndex)
    {
      if(gsl::not_null {
           pybind11::globals()["level_sequence"][pybind11::cast(levelSequenceIndex)]
             .cast<engine::script::LevelSequenceItem*>()
         }->isLevel(filename))
        break;
    }
    Expects(levelSequenceIndex < levelSequenceLength);
    loadSlot = slot;
    doLoad = true;
    mode = Mode::Game;
  };

  while(true)
  {
    std::pair<engine::RunResult, std::optional<size_t>> runResult;
    switch(mode)
    {
    case Mode::Title:
      Expects(!doLoad);
      runResult = engine.runLevelSequenceItem(
        *gsl::not_null{pybind11::globals()["title_menu"].cast<engine::script::LevelSequenceItem*>()});
      break;
    case Mode::Gym:
      Expects(!doLoad);
      runResult = engine.runLevelSequenceItem(
        *gsl::not_null{pybind11::globals()["lara_home"].cast<engine::script::LevelSequenceItem*>()});
      break;
    case Mode::Game:
      if(doLoad)
        runResult = engine.runLevelSequenceItemFromSave(
          *gsl::not_null{pybind11::globals()["level_sequence"][pybind11::cast(levelSequenceIndex)]
                           .cast<engine::script::LevelSequenceItem*>()},
          loadSlot);
      else
        runResult = engine.runLevelSequenceItem(
          *gsl::not_null{pybind11::globals()["level_sequence"][pybind11::cast(levelSequenceIndex)]
                           .cast<engine::script::LevelSequenceItem*>()});
      break;
    }

    loadSlot.reset();
    doLoad = false;

    switch(mode)
    {
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
        if(levelSequenceIndex >= levelSequenceLength)
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
