#pragma once

#include "quantity.h"

#include <iosfwd>

namespace qs
{
template<typename Unit, typename Type>
std::ostream& operator<<(std::ostream& o, quantity<Unit, Type> q)
{
  return o << q.get() << Unit::suffix();
}
} // namespace qs
