#pragma once

#include "types.h"

#include <bitset>

namespace engine::floordata
{
extern std::bitset<16> getSecretsMask(const FloorDataValue* floorData);
}
