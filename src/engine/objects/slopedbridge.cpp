#include "slopedbridge.h"

#include "serialization/serialization.h"

namespace engine::objects
{
void SlopedBridge::serialize(const serialization::Serializer<world::World>& ser)
{
  ModelObject::serialize(ser);
  ser(S_NV("flatness", m_flatness));
}
} // namespace engine::objects
