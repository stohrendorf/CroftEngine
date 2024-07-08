#pragma once

#include "engine/objects/object.h"
#include "engine/world/world.h"
#include "serialization.h"

#include <functional>
#include <gsl/gsl-lite.hpp>
#include <memory>

namespace serialization
{
template<typename T>
struct ObjectReference final
{
  ObjectReference(const ObjectReference<T>&) = delete;
  ObjectReference(ObjectReference<T>&&) = delete;
  ~ObjectReference() = default;
  void operator=(ObjectReference<T>&&) = delete;
  void operator=(const ObjectReference<T>&) = delete;

  static_assert(std::is_base_of_v<engine::objects::Object, T>);
  std::reference_wrapper<std::shared_ptr<T>> ptr;

  explicit ObjectReference(const std::reference_wrapper<const std::shared_ptr<T>>& ptr)
      : ptr{const_cast<std::shared_ptr<T>&>(ptr.get())}
  {
  }

  explicit ObjectReference(std::reference_wrapper<std::shared_ptr<T>>&& ptr)
      : ptr{std::move(ptr)}
  {
  }

  void serialize(const Serializer<engine::world::World>& ser) const
  {
    if(ptr.get() == nullptr)
    {
      ser.setNull();
    }
    else
    {
      ser.tag("objectref");
      for(const auto& [objId, obj] : ser.context->getObjectManager().getObjects())
      {
        if(obj.get() == ptr.get())
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

  void deserialize(const Deserializer<engine::world::World>& ser)
  {
    if(ser.isNull())
    {
      ptr.get() = nullptr;
    }
    else
    {
      ser << [ptr = ptr](const Deserializer<engine::world::World>& ser)
      {
        ser.tag("objectref");
        engine::ObjectId id = 0;
        ser(S_NV("id", id));
        auto tmp = ser.context->getObjectManager().getObjects().at(id).get();
        gsl_Assert(tmp != nullptr);
        ptr.get() = std::dynamic_pointer_cast<T>(tmp);
      };
    }
  }
};
} // namespace serialization
