#include "slopedbridge.h"

#include "modelobject.h"
#include "serialization/serialization.h"

namespace engine::objects
{
void SlopedBridge::serialize(const serialization::Serializer<world::World>& ser) const
{
  ModelObject::serialize(ser);
  ser(S_NV("flatness", m_flatness));
}

void SlopedBridge::deserialize(const serialization::Deserializer<world::World>& ser)
{
  ModelObject::deserialize(ser);
  ser(S_NV("flatness", m_flatness));
}
} // namespace engine::objects
