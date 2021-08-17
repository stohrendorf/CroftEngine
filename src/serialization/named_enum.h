#pragma once

#include "serialization.h"

namespace serialization
{
template<typename T, typename Converter>
struct NamedEnum
{
  T value;

  // NOLINTNEXTLINE(google-explicit-constructor)
  NamedEnum(T value = T{})
      : value{value}
  {
  }

  template<typename TContext>
  void load(const Serializer<TContext>& ser)
  {
    ser.tag(Converter::name());

    if(!ser.node.has_val())
      SERIALIZER_EXCEPTION("cannot get enum value from non-value node");

    std::string name;
    ser.node >> name;
    try
    {
      value = Converter::fromString(name);
    }
    catch(std::domain_error&)
    {
      SERIALIZER_EXCEPTION("invalid enum value");
    }
  }

  template<typename TContext>
  static NamedEnum<T, Converter> create(const Serializer<TContext>& ser)
  {
    NamedEnum<T, Converter> result{};
    result.load(ser);
    return result;
  }

  template<typename TContext>
  void save(const Serializer<TContext>& ser) const
  {
    ser.tag(Converter::name());
    ser.node << toString(value);
  }

  // NOLINTNEXTLINE(google-explicit-constructor)
  operator T() const noexcept
  {
    return value;
  }

  NamedEnum<T, Converter>& operator=(T e)
  {
    value = e;
    return *this;
  }
};
} // namespace serialization
