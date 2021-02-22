#include "player.h"

#include "serialization/quantity.h"
#include "serialization/serialization.h"

namespace engine
{
void Player::serialize(const serialization::Serializer<World>& ser)
{
  ser(S_NV("inventory", m_inventory), S_NV("laraHealth", laraHealth));
}
} // namespace engine
