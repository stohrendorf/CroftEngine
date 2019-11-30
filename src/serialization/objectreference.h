#pragma once

#include "engine/engine.h"
#include "ptr.h"

namespace serialization
{
struct ObjectReference
{
  std::shared_ptr<engine::objects::Object>& ptr;

  explicit ObjectReference(std::shared_ptr<engine::objects::Object>& ptr)
      : ptr{ptr}
  {
  }

  void save(const Serializer& ser)
  {
    if(ptr == nullptr)
    {
      ser.node = YAML::Node{};
    }
    else
    {
      for(const auto& obj : ser.engine.getObjects())
      {
        if(obj.second.get() == ptr)
        {
          engine::ObjectId tmp = obj.first;
          ser("id", tmp);
        }
      }
    }
  }

  void load(const Serializer& ser)
  {
    if(ser.node.IsNull())
    {
      ptr = nullptr;
    }
    else
    {
      ser.lazy([pptr = &ptr](const Serializer& ser) {
        engine::ObjectId id = 0;
        ser("id", id);
        *pptr = ser.engine.getObjects().at(id);
      });
    }
  }
};

} // namespace serialization
