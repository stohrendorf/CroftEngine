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
  size_t levelIndex = 0;
  enum class Mode
  {
    Title,
    Gym,
    Game
  };
  auto mode = Mode::Title;
  while(true)
  {
    switch(mode)
    {
    case Mode::Title:
      switch(engine.runLevelSequenceItem(
        *gsl::not_null{pybind11::globals()["title_menu"].cast<engine::script::LevelSequenceItem*>()}))
      {
      case engine::RunResult::ExitApp: return EXIT_SUCCESS;
      case engine::RunResult::NextLevel:
        levelIndex = 0;
        mode = Mode::Game;
        break;
      case engine::RunResult::TitleLevel: mode = Mode::Title; break;
      case engine::RunResult::LaraHomeLevel: mode = Mode::Gym; break;
      }
      break;
    case Mode::Gym:
      switch(engine.runLevelSequenceItem(
        *gsl::not_null{pybind11::globals()["lara_home"].cast<engine::script::LevelSequenceItem*>()}))
      {
      case engine::RunResult::ExitApp: return EXIT_SUCCESS;
      case engine::RunResult::NextLevel: mode = Mode::Title; break;
      case engine::RunResult::TitleLevel: mode = Mode::Title; break;
      case engine::RunResult::LaraHomeLevel: mode = Mode::Gym; break;
      }
      break;
    case Mode::Game:
      switch(engine.runLevelSequenceItem(*gsl::not_null{
        pybind11::globals()["level_sequence"][pybind11::cast(levelIndex)].cast<engine::script::LevelSequenceItem*>()}))
      {
      case engine::RunResult::ExitApp: return EXIT_SUCCESS;
      case engine::RunResult::NextLevel: ++levelIndex; break;
      case engine::RunResult::TitleLevel: mode = Mode::Title; break;
      case engine::RunResult::LaraHomeLevel: mode = Mode::Gym; break;
      }
      break;
    }
  }
}
