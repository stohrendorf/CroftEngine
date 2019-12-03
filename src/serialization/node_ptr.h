#pragma once

#include "engine/engine.h"
#include "gsl-lite.hpp"
#include "render/scene/node.h"
#include "serialization.h"

namespace serialization
{
inline void save(const gsl::not_null<std::shared_ptr<render::scene::Node>>& node, const Serializer& ser)
{
  ser.tag("renderableref");
  if(node->getRenderable() == nullptr)
  {
    ser.node = YAML::Node{};
  }
  else
  {
    if(const auto idx = ser.engine.indexOfModel(node->getRenderable()))
      ser.node = *idx;
    else
      ser.node = YAML::Node{};
  }
}

inline void load(gsl::not_null<std::shared_ptr<render::scene::Node>>& node, const Serializer& ser)
{
  ser.tag("renderableref");
  if(ser.node.IsNull())
  {
    node->setRenderable(nullptr);
  }
  else
  {
    node->setRenderable(ser.engine.getModel(ser.node.as<size_t>()).get());
  }
}

} // namespace serialization
