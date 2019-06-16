#include "engine/engine.h"

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/stacktrace.hpp>

#ifndef BOOST_MSVC
#    include <signal.h>

void stacktrace_handler(int signum)
{
    ::signal(signum, SIG_DFL);
    std::cerr << "Signal " << signum << " caught; stacktrace:\n" << boost::stacktrace::stacktrace();
    ::raise(SIGABRT);
}
#endif

gsl_api void gsl::fail_fast_assert_handler(char const* const expression,
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
std::terminate_handler oldTerminateHandler = nullptr;

void terminateHandler()
{
    BOOST_LOG_TRIVIAL(error) << "Abnormal termination. Stacktrace:\n" << boost::stacktrace::stacktrace();

    if(oldTerminateHandler != nullptr)
        oldTerminateHandler();
}
} // namespace

int main()
{
#ifndef BOOST_MSVC
    ::signal(SIGSEGV, &stacktrace_handler);
    ::signal(SIGABRT, &stacktrace_handler);
#endif

    oldTerminateHandler = std::set_terminate(&terminateHandler);

#ifdef NDEBUG
    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
#endif

    try
    {
        engine::Engine engine;
        engine.run();

        return EXIT_SUCCESS;
    }
    catch(std::exception& ex)
    {
        BOOST_LOG_TRIVIAL(error) << "Error: " << ex.what();
        return EXIT_FAILURE;
    }
    catch(...)
    {
        BOOST_LOG_TRIVIAL(error) << "Unexpected error";
        return EXIT_FAILURE;
    }
}
