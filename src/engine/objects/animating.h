#pragma once

#include "modelobject.h"

namespace engine::objects
{
class Animating final : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(Animating, true)

  void update() override;
};
} // namespace engine::objects
