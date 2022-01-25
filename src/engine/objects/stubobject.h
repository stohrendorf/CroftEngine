#pragma once

#include "modelobject.h"

namespace engine::objects
{
class StubObject final : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(StubObject, false, false)
};
} // namespace engine::objects
