#pragma once

#include "io/sdlreader.h"

#include <gsl/gsl>

namespace loader
{
#pragma pack(push, 1)


struct BoundingBox
{
    int16_t minX{0}, maxX{0};
    int16_t minY{0}, maxY{0};
    int16_t minZ{0}, maxZ{0};

    explicit BoundingBox() = default;

    BoundingBox(const BoundingBox& a, const BoundingBox& b, float bias)
            : minX{static_cast<int16_t>(a.minX * (1 - bias) + b.minX * bias)}
            , maxX{static_cast<int16_t>(a.maxX * (1 - bias) + b.maxX * bias)}
            , minY{static_cast<int16_t>(a.minY * (1 - bias) + b.minY * bias)}
            , maxY{static_cast<int16_t>(a.maxY * (1 - bias) + b.maxY * bias)}
            , minZ{static_cast<int16_t>(a.minZ * (1 - bias) + b.minZ * bias)}
            , maxZ{static_cast<int16_t>(a.maxZ * (1 - bias) + b.maxZ * bias)}
    {
    }

    core::TRCoordinates getCenter() const
    {
        return {(minX + maxX) / 2, (minY + maxY) / 2, (minZ + maxZ) / 2};
    }
};


struct AnimFrame
{
    struct Vec
    {
        int16_t x, y, z;

        glm::vec3 toGl() const noexcept
        {
            return glm::vec3( x, -y, -z );
        }
    };


    BoundingBox bbox;
    Vec pos;
    uint16_t numValues;

    gsl::span<const uint32_t> getAngleData() const noexcept
    {
        const auto begin = reinterpret_cast<const uint32_t*>(this + 1);
        return gsl::make_span( begin, numValues );
    }

    const AnimFrame* next() const
    {
        const auto begin = reinterpret_cast<const uint32_t*>(this + 1);
        const auto end = begin + numValues;
        return reinterpret_cast<const AnimFrame*>(end);
    }

    const AnimFrame* next(size_t n) const
    {
        auto result = this;
        while( n-- )
            result = result->next();
        return result;
    }
};


static_assert( sizeof( AnimFrame ) == 20, "AnimFrame has wrong size" );

#pragma pack(pop)


struct Animation
{
    uint32_t poseDataOffset; // byte offset into Frames[] (divide by 2 for Frames[i])

    const AnimFrame* poseData = nullptr;

    uint8_t segmentLength; // Slowdown factor of this animation
    uint8_t poseDataSize; // number of bit16's in Frames[] used by this animation
    uint16_t state_id;

    int32_t speed;
    int32_t accelleration;

    int32_t lateralSpeed; // new in TR4 -->
    int32_t lateralAccelleration; // lateral speed and acceleration.

    uint16_t firstFrame; // first frame in this animation
    uint16_t lastFrame; // last frame in this animation (numframes = (End - Start) + 1)
    uint16_t nextAnimation;
    uint16_t nextFrame;

    uint16_t transitionsCount;
    uint16_t transitionsIndex; // offset into StateChanges[]
    uint16_t animCommandCount; // How many of them to use.
    uint16_t animCommandIndex; // offset into AnimCommand[]

    constexpr size_t getKeyframeCount() const
    {
        return (lastFrame - firstFrame + segmentLength) / segmentLength;
    }

    constexpr size_t getFrameCount() const
    {
        return lastFrame - firstFrame + 1;
    }

    /// \brief reads an animation definition.
    static std::unique_ptr<Animation> readTr1(io::SDLReader& reader)
    {
        return read( reader, false );
    }

    static std::unique_ptr<Animation> readTr4(io::SDLReader& reader)
    {
        return read( reader, true );
    }

private:
    static std::unique_ptr<Animation> read(io::SDLReader& reader, bool withLateral)
    {
        std::unique_ptr<Animation> animation{new Animation()};
        animation->poseDataOffset = reader.readU32();
        animation->segmentLength = reader.readU8();
        if( animation->segmentLength == 0 )
            animation->segmentLength = 1;
        animation->poseDataSize = reader.readU8();
        animation->state_id = reader.readU16();

        animation->speed = reader.readI32();
        animation->accelleration = reader.readI32();
        if( withLateral )
        {
            animation->lateralSpeed = reader.readI32();
            animation->lateralAccelleration = reader.readI32();
        }
        else
        {
            animation->lateralSpeed = 0;
            animation->lateralAccelleration = 0;
        }

        animation->firstFrame = reader.readU16();
        animation->lastFrame = reader.readU16();
        animation->nextAnimation = reader.readU16();
        animation->nextFrame = reader.readU16();

        animation->transitionsCount = reader.readU16();
        animation->transitionsIndex = reader.readU16();
        animation->animCommandCount = reader.readU16();
        animation->animCommandIndex = reader.readU16();
        return animation;
    }
};


/** \brief State Change.
*
* Each one contains the state to change to and which animation dispatches
* to use; there may be more than one, with each separate one covering a different
* range of frames.
*/
struct Transitions
{
    uint16_t stateId;
    uint16_t transitionCaseCount; // number of ranges (seems to always be 1..5)
    uint16_t firstTransitionCase; // Offset into AnimDispatches[]

    /// \brief reads an animation state change.
    static std::unique_ptr<Transitions> read(io::SDLReader& reader)
    {
        std::unique_ptr<Transitions> state_change{new Transitions()};
        state_change->stateId = reader.readU16();
        state_change->transitionCaseCount = reader.readU16();
        state_change->firstTransitionCase = reader.readU16();
        return state_change;
    }
};


/** \brief Animation Dispatch.
*
* This specifies the next animation and frame to use; these are associated
* with some range of frames. This makes possible such specificity as one
* animation for left foot forward and another animation for right foot forward.
*/
struct TransitionCase
{
    uint16_t firstFrame; // Lowest frame that uses this range
    uint16_t lastFrame; // Highest frame (+1?) that uses this range
    uint16_t targetAnimation; // Animation to dispatch to
    uint16_t targetFrame; // Frame offset to dispatch to

    /// \brief reads an animation dispatch.
    static std::unique_ptr<TransitionCase> read(io::SDLReader& reader)
    {
        std::unique_ptr<TransitionCase> anim_dispatch{new TransitionCase()};
        anim_dispatch->firstFrame = reader.readU16();
        anim_dispatch->lastFrame = reader.readU16();
        anim_dispatch->targetAnimation = reader.readU16();
        anim_dispatch->targetFrame = reader.readU16();
        return anim_dispatch;
    }
};


struct SkeletalModelType
{
    uint32_t typeId; // Item Identifier (matched in Items[])
    uint16_t nmeshes; // number of meshes in this object
    uint16_t frame_number; // starting mesh (offset into MeshPointers[])
    uint32_t bone_index; // offset into MeshTree[]
    uint32_t pose_data_offset; // byte offset into Frames[] (divide by 2 for Frames[i])
    uint16_t anim_index; // offset into Animations[]

    const AnimFrame* frame_base = nullptr;

    /** \brief reads a moveable definition.
    *
    * some sanity checks get done which throw a exception on failure.
    * frame_offset needs to be corrected later in TR_Level::read_tr_level.
    */
    static std::unique_ptr<SkeletalModelType> readTr1(io::SDLReader& reader)
    {
        std::unique_ptr<SkeletalModelType> moveable{new SkeletalModelType()};
        moveable->typeId = reader.readU32();
        moveable->nmeshes = reader.readU16();
        moveable->frame_number = reader.readU16();
        moveable->bone_index = reader.readU32();
        moveable->pose_data_offset = reader.readU32();
        moveable->anim_index = reader.readU16();
        return moveable;
    }

    static std::unique_ptr<SkeletalModelType> readTr5(io::SDLReader& reader)
    {
        std::unique_ptr<SkeletalModelType> moveable = readTr1( reader );
        if( reader.readU16() != 0xFFEF )
            BOOST_LOG_TRIVIAL( warning ) << "TR5 Moveable: filler has wrong value";
        return moveable;
    }
};
}
