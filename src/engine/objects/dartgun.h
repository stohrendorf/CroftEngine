#pragma once

#include "modelobject.h"

namespace engine::objects
{
class DartGun final : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(DartGun, true)

  void update() override;
};
} // namespace engine::objects
