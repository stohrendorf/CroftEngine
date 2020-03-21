#pragma once

#include "engine/engine.h"
#include "optional.h"
#include "ptr.h"

namespace serialization
{
void save(std::shared_ptr<loader::file::RenderMeshData>& mesh, const Serializer& ser)
{
  if(mesh == nullptr)
  {
    ser.node = YAML::Node{YAML::NodeType::Null};
    return;
  }

  ser.tag("mesh");
  uint32_t idx = 0;
  for(const auto& existing : ser.engine.getMeshes())
  {
    if(existing.meshData == mesh)
    {
      ser.node = idx;
      return;
    }
    ++idx;
  }
  Expects(false);
}

void load(std::shared_ptr<loader::file::RenderMeshData>& data, const Serializer& ser)
{
  if(ser.node.IsNull())
  {
    data = nullptr;
    return;
  }

  ser.tag("mesh");
  data = ser.engine.getRenderMesh(ser.node.as<uint32_t>());
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
