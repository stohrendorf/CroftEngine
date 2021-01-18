#include "engine/engine.h"

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
  while(true)
  {
    const auto result = engine.runLevelSequenceItem(levelIndex);
    switch(result)
    {
    case engine::RunResult::ExitApp: return EXIT_SUCCESS;
    case engine::RunResult::NextLevel:
      if(levelIndex == 0)
        levelIndex = 2;
      else if(levelIndex == 1)
        levelIndex = 0;
      else
        ++levelIndex;
      break;
    case engine::RunResult::TitleLevel: levelIndex = 0; break;
    case engine::RunResult::LaraHomeLevel: levelIndex = 1; break;
    }
  }

  return EXIT_SUCCESS;
}
