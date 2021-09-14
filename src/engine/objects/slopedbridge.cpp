#include "slopedbridge.h"

#include "modelobject.h"
#include "serialization/serialization.h"

#include <exception>

namespace engine::objects
{
void SlopedBridge::serialize(const serialization::Serializer<world::World>& ser)
{
  ModelObject::serialize(ser);
  ser(S_NV("flatness", m_flatness));
}
} // namespace engine::objects
