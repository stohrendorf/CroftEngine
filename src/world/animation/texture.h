#pragma once

#include "util/helpers.h"

#include <vector>

namespace world
{
namespace animation
{
// Animated texture types
enum class TextureAnimationType
{
    Forward,
    Backward,
    Reverse
};

/*
*  Animated sequence. Used globally with animated textures to refer its parameters and frame numbers.
*/
struct TextureAnimationKeyFrame
{
    irr::core::vector2df coordinateTransform[2];
    irr::core::vector2df move;
    size_t textureIndex;
};

struct TextureAnimationSequence
{
    bool uvrotate = false;   //!< UVRotate mode flag.
    irr::f32 uvrotateSpeed;   // Speed of UVRotation, in seconds.
    irr::f32 uvrotateMax;     // Reference value used to restart rotation.

    bool frame_lock = false; //!< Single frame mode. Needed for TR4-5 compatible UVRotate.

    TextureAnimationType textureType = TextureAnimationType::Forward;
    bool reverse = false;    // Used only with type 2 to identify current animation direction.
    util::Duration frameTime = util::Duration::zero(); // Time passed since last frame update.
    size_t currentFrame = 0;    // Current frame for this sequence.
    util::Duration timePerFrame = util::MilliSeconds(50); // For types 0-1, specifies framerate, for type 3, should specify rotation speed.

    std::vector<TextureAnimationKeyFrame> keyFrames;
    std::vector<size_t> textureIndices; // Offset into anim textures frame list.
};
}
}
