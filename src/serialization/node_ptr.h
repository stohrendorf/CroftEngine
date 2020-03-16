#pragma once

#include "engine/engine.h"
#include "render/scene/node.h"
#include "serialization.h"

#include <gsl-lite.hpp>

namespace serialization
{
inline void save(const gsl::not_null<std::shared_ptr<render::scene::Node>>& node, const Serializer& ser)
{
  if(node->getRenderable() == nullptr)
  {
    ser.node = YAML::Node{YAML::NodeType::Null};
  }
  else
  {
    if(const auto idx = ser.engine.indexOfModel(node->getRenderable()))
    {
      ser.tag("renderableref");
      ser.node = *idx;
    }
    else
      ser.node = YAML::Node{YAML::NodeType::Null};
  }
}

inline void load(gsl::not_null<std::shared_ptr<render::scene::Node>>& node, const Serializer& ser)
{
  if(ser.node.IsNull())
  {
    node->setRenderable(nullptr);
  }
  else
  {
    ser.tag("renderableref");
    node->setRenderable(ser.engine.getRenderMesh(ser.node.as<size_t>()).get());
  }
}

} // namespace serialization
