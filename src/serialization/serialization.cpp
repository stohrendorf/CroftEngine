#include "serialization.h"

#include <stdexcept>
#include <string>

#ifndef NDEBUG
#  include <boost/log/trivial.hpp>
#endif

namespace serialization
{
Exception::Exception(const std::string& msg)
    : std::runtime_error{msg}
{
#ifndef NDEBUG
  BOOST_LOG_TRIVIAL(error) << "Serialization exception: " << msg;
#endif
}
} // namespace serialization
