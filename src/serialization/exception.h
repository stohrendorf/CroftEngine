#pragma once

#include <boost/throw_exception.hpp>
#include <stdexcept>
#include <string>

namespace serialization
{
class Exception : public std::runtime_error
{
public:
  explicit Exception(const std::string& msg);
};

#define SERIALIZER_EXCEPTION(...) BOOST_THROW_EXCEPTION(::serialization::Exception(__VA_ARGS__))
} // namespace serialization
