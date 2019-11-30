#include "floordata.h"

#include "serialization/bitset.h"

namespace engine::floordata
{
void ActivationState::serialize(const serialization::Serializer& ser)
{
  ser(S_NVP(m_oneshot), S_NVP(m_inverted), S_NVP(m_locked), S_NVP(m_activationSet));
}
} // namespace engine::floordata
