#pragma once

#include "engine/world/world.h"
#include "optional.h"
#include "ptr.h"
#include "serialization.h"

namespace serialization
{
// cppcheck-suppress constParameter
void serialize(const std::shared_ptr<engine::world::RenderMeshData>& mesh, const Serializer<engine::world::World>& ser)
{
  if(mesh == nullptr)
  {
    ser.setNull();
    return;
  }

  ser.tag("mesh");
  uint32_t idx = 0;
  for(const auto& existing : ser.context->getMeshes())
  {
    if(existing.meshData == mesh)
    {
      ser.node << idx;
      return;
    }
    ++idx;
  }
  gsl_Assert(false);
}

void deserialize(std::shared_ptr<engine::world::RenderMeshData>& data, const Deserializer<engine::world::World>& ser)
{
  if(ser.isNull())
  {
    data = nullptr;
    return;
  }

  ser.tag("mesh");
  uint32_t tmp{};
  ser.node >> tmp;
  data = ser.context->getRenderMesh(tmp);
}

std::shared_ptr<engine::world::RenderMeshData> create(const TypeId<std::shared_ptr<engine::world::RenderMeshData>>&,
                                                      const Deserializer<engine::world::World>& ser)
{
  std::shared_ptr<engine::world::RenderMeshData> tmp;
  deserialize(tmp, ser);
  return tmp;
}
} // namespace serialization
