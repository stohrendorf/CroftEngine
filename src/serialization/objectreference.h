#pragma once

#include "engine/world/world.h"
#include "ptr.h"
#include "serialization.h"

namespace serialization
{
template<typename T>
struct SerializingObjectReference final
{
  static_assert(std::is_base_of_v<engine::objects::Object, T>);
  std::shared_ptr<T> ptr;

  explicit SerializingObjectReference(const std::shared_ptr<T>& ptr)
      : ptr{ptr}
  {
  }

  void serialize(const Serializer<engine::world::World>& ser) const
  {
    if(ptr == nullptr)
    {
      ser.setNull();
    }
    else
    {
      ser.tag("objectref");
      for(const auto& [objId, obj] : ser.context.getObjectManager().getObjects())
      {
        if(obj.get() == ptr)
        {
          engine::ObjectId tmp = objId;
          ser(S_NV("id", tmp));
          return;
        }
      }

      // this may happen if the object was killed, thus rendering this reference invalid
      ser.setNull();
    }
  }
};

template<typename T>
struct DeserializingObjectReference final
{
  static_assert(std::is_base_of_v<engine::objects::Object, T>);
  std::shared_ptr<T>& ptr;

  explicit DeserializingObjectReference(std::shared_ptr<T>& ptr)
      : ptr{ptr}
  {
  }

  void deserialize(const Deserializer<engine::world::World>& ser)
  {
    if(ser.isNull())
    {
      ptr = nullptr;
    }
    else
    {
      ser << [pptr = &ptr](const Deserializer<engine::world::World>& ser)
      {
        ser.tag("objectref");
        engine::ObjectId id = 0;
        ser(S_NV("id", id));
        auto tmp = ser.context.getObjectManager().getObjects().at(id).get();
        gsl_Assert(tmp != nullptr);
        *pptr = std::dynamic_pointer_cast<T>(tmp);
      };
    }
  }
};
} // namespace serialization
