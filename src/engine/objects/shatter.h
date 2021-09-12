#pragma once

#include "core/units.h"

#include <bitset>

namespace engine::objects
{
class ModelObject;

extern bool shatterModel(ModelObject& object, const std::bitset<32>& meshMask, const core::Length& damageRadius);
} // namespace engine::objects
