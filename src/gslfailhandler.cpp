#include <boost/log/trivial.hpp>
#include <boost/stacktrace.hpp>
#include <boost/throw_exception.hpp>
#include <gsl-lite.hpp>

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
