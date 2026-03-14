#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <gsl-lite/gsl-lite.hpp>

[[maybe_unused]] void gsl_lite::fail_fast_assert_handler(char const* const expression,
                                                         char const* const message,
                                                         char const* const file,
                                                         const int line)
{
  BOOST_LOG_TRIVIAL(fatal) << "Expectation failed at " << file << ":" << line;
  BOOST_LOG_TRIVIAL(fatal) << "  - expression " << expression;
  BOOST_LOG_TRIVIAL(fatal) << "  - message " << message;
  BOOST_THROW_EXCEPTION(gsl_lite::fail_fast(message));
}