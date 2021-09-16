#pragma once

#include <boost/throw_exception.hpp>
#include <gsl/gsl-lite.hpp>
#include <stdexcept>
#include <string>

namespace serialization
{
class Exception : public std::runtime_error
{
public:
  explicit Exception(const std::string& msg)
      : Exception{msg.c_str()}
  {
  }

  explicit Exception(gsl::czstring msg);
};

#define SERIALIZER_EXCEPTION(msg) BOOST_THROW_EXCEPTION(::serialization::Exception{msg})
} // namespace serialization
