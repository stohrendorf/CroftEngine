#pragma once

#include "quantity.h"

#include <iosfwd>

namespace qs
{
template<typename Unit, typename Type>
inline std::ostream& operator<<(std::ostream& o, quantity <Unit, Type> q)
{
    return o << q.get() << Unit::suffix();
}
}
