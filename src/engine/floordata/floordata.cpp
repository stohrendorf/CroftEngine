#include "floordata.h"

#include "serialization/bitset.h"

namespace engine::floordata
{
void ActivationState::serialize(const serialization::Serializer& ser)
{
  ser(S_NV("oneshot", m_oneshot),
      S_NV("inverted", m_inverted),
      S_NV("locked", m_locked),
      S_NV("activationSet", m_activationSet));
}
} // namespace engine::floordata
