#pragma once

#include <irrlicht.h>

namespace util
{
inline irr::core::quaternion trRotationToQuat(const irr::core::vector3df& rotation)
{
    irr::core::quaternion v;
    v *= irr::core::quaternion().fromAngleAxis(irr::core::degToRad(rotation.Z), {0,0,1});
    v *= irr::core::quaternion().fromAngleAxis(irr::core::degToRad(rotation.X), {1,0,0});
    v *= irr::core::quaternion().fromAngleAxis(irr::core::degToRad(rotation.Y), {0,1,0});
    return v;
}

constexpr float auToDeg(int16_t au)
{
    return au / 65536.0f * 360;
}
} // namespace util
