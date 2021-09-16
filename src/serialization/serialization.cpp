#include "serialization.h"

#include <stdexcept>

namespace serialization
{
Exception::Exception(const gsl::czstring msg)
    : std::runtime_error{msg}
{
#ifndef NDEBUG
  BOOST_LOG_TRIVIAL(error) << "Serialization exception: " << msg;
#endif
}
} // namespace serialization
