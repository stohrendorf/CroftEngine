#include "serialization.h"

#include <boost/algorithm/string/replace.hpp>
#include <boost/stacktrace.hpp>

namespace serialization
{
Exception::Exception(const gsl::czstring msg)
    : std::runtime_error{msg}
{
  BOOST_LOG_TRIVIAL(fatal) << "Serialization exception: " << msg;
  BOOST_LOG_TRIVIAL(fatal) << "Stacktrace:\n" << boost::stacktrace::stacktrace();
}
} // namespace serialization
