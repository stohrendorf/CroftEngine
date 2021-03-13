#pragma once

#include "engine/world/world.h"
#include "ptr.h"
#include "serialization.h"

#include <type_traits>

namespace serialization
{
template<typename T>
struct ObjectReference final
{
  static_assert(std::is_base_of_v<engine::objects::Object, T>);
  std::shared_ptr<T>& ptr;

  explicit ObjectReference(std::shared_ptr<T>& ptr)
      : ptr{ptr}
  {
  }

  void save(const Serializer<engine::world::World>& ser) const
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
        }
      }
    }
  }

  void load(const Serializer<engine::world::World>& ser)
  {
    if(ser.isNull())
    {
      ptr = nullptr;
    }
    else
    {
      ser.lazy([pptr = &ptr](const Serializer<engine::world::World>& ser) {
        ser.tag("objectref");
        engine::ObjectId id = 0;
        ser(S_NV("id", id));
        auto tmp = ser.context.getObjectManager().getObjects().at(id).get();
        Expects(tmp != nullptr);
        *pptr = std::dynamic_pointer_cast<T>(tmp);
      });
    }
  }
};

} // namespace serialization
