#pragma once

#include "loader/datatypes.h"

namespace world
{
namespace core
{
struct Light
{
    irr::core::vector3df position;
    irr::video::SColorf color;

    irr::f32 inner;
    irr::f32 outer;
    irr::f32 length;
    irr::f32 cutoff;

    irr::f32 falloff;

    loader::LightType light_type;
};
} // namespace core
} // namespace world
