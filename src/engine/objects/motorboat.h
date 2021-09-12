#pragma once

#include "modelobject.h"

namespace engine::objects
{
class Motorboat final : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(Motorboat, true);

  void update() override;
};
} // namespace engine::objects
