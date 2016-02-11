#pragma once

#include "audio.h"

#include <irrlicht.h>

namespace audio
{
// Audio emitter (aka SoundSource) structure.
struct Emitter
{
    size_t      emitter_index;  // Unique emitter index.
    SoundId     soundId;
    irr::core::vector3df position;    // Vector coordinate.
    uint16_t    flags;          // Flags - MEANING UNKNOWN!!!
};
} // namespace audio
