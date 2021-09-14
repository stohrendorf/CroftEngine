#pragma once

#include "modelobject.h"

namespace engine::world
{
struct SkeletalModelType;
}

namespace engine::objects
{
class AtlanteanLava final : public ModelObject
{
public:
  MODELOBJECT_DEFAULT_CONSTRUCTORS(AtlanteanLava, true)

  void update() override;
};
} // namespace engine::objects
