#pragma once

#include "engine/world/world.h"
#include "optional.h"
#include "ptr.h"
#include "serialization.h"

namespace serialization
{
// cppcheck-suppress constParameter
void save(const std::shared_ptr<engine::world::RenderMeshData>& mesh, const Serializer<engine::world::World>& ser)
{
  if(mesh == nullptr)
  {
    ser.setNull();
    return;
  }

  ser.tag("mesh");
  uint32_t idx = 0;
  for(const auto& existing : ser.context.getMeshes())
  {
    if(existing.meshData == mesh)
    {
      ser.node << idx;
      return;
    }
    ++idx;
  }
  Expects(false);
}

void load(std::shared_ptr<engine::world::RenderMeshData>& data, const Serializer<engine::world::World>& ser)
{
  if(ser.isNull())
  {
    data = nullptr;
    return;
  }

  ser.tag("mesh");
  uint32_t tmp{};
  ser.node >> tmp;
  data = ser.context.getRenderMesh(tmp);
}

std::shared_ptr<engine::world::RenderMeshData> create(const TypeId<std::shared_ptr<engine::world::RenderMeshData>>&,
                                                      const Serializer<engine::world::World>& ser)
{
  Expects(ser.loading);
  std::shared_ptr<engine::world::RenderMeshData> tmp;
  load(tmp, ser);
  return tmp;
}
} // namespace serialization
