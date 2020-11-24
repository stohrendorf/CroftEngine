#pragma once

#include "engine/world.h"
#include "optional.h"
#include "ptr.h"

namespace serialization
{
// cppcheck-suppress constParameter
void save(const std::shared_ptr<loader::file::RenderMeshData>& mesh, const Serializer& ser)
{
  if(mesh == nullptr)
  {
    ser.setNull();
    return;
  }

  ser.tag("mesh");
  uint32_t idx = 0;
  for(const auto& existing : ser.world.getMeshes())
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

void load(std::shared_ptr<loader::file::RenderMeshData>& data, const Serializer& ser)
{
  if(ser.isNull())
  {
    data = nullptr;
    return;
  }

  ser.tag("mesh");
  uint32_t tmp{};
  ser.node >> tmp;
  data = ser.world.getRenderMesh(tmp);
}

std::shared_ptr<loader::file::RenderMeshData> create(const TypeId<std::shared_ptr<loader::file::RenderMeshData>>&,
                                                     const Serializer& ser)
{
  Expects(ser.loading);
  std::shared_ptr<loader::file::RenderMeshData> tmp;
  load(tmp, ser);
  return tmp;
}
} // namespace serialization
