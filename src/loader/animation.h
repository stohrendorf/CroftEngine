#pragma once

#include "io/sdlreader.h"
#include "engine/items_tr1.h"
#include "core/vec.h"

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

    core::TRVec getCenter() const
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
        const auto next = reinterpret_cast<const AnimFrame*>(end);
        Expects( next->numValues == numValues );
        return next;
    }

    const AnimFrame* next(size_t n) const
    {
        auto result = this;
        while( n-- )
        {
            result = result->next();
            BOOST_ASSERT( result->numValues == numValues );
        }
        return result;
    }
};


static_assert( sizeof( AnimFrame ) == 20, "AnimFrame has wrong size" );

#pragma pack(pop)

struct Transitions;


struct Animation
{
    uint32_t poseDataOffset; // byte offset into Frames[] (divide by 2 for Frames[i])

    const AnimFrame* frames = nullptr;

    uint8_t segmentLength; // Slowdown factor of this animation
    uint8_t poseDataSize; // number of bit16's in Frames[] used by this animation
    uint16_t state_id;

    int32_t speed;
    int32_t accelleration;

    int32_t lateralSpeed; // new in TR4 -->
    int32_t lateralAccelleration; // lateral speed and acceleration.

    uint16_t firstFrame; // first frame in this animation
    uint16_t lastFrame; // last frame in this animation (numframes = (End - Start) + 1)
    uint16_t nextAnimationIndex;
    uint16_t nextFrame;

    uint16_t transitionsCount;
    uint16_t transitionsIndex; // offset into StateChanges[]
    uint16_t animCommandCount; // How many of them to use.
    uint16_t animCommandIndex; // offset into AnimCommand[]

    const Animation* nextAnimation = nullptr;
    gsl::span<const Transitions> transitions{};

    constexpr size_t getKeyframeCount() const
    {
        return (lastFrame - firstFrame + segmentLength) / segmentLength;
    }

    constexpr size_t getFrameCount() const
    {
        return lastFrame - firstFrame + 1;
    }

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
        animation->nextAnimationIndex = reader.readU16();
        animation->nextFrame = reader.readU16();

        animation->transitionsCount = reader.readU16();
        animation->transitionsIndex = reader.readU16();
        animation->animCommandCount = reader.readU16();
        animation->animCommandIndex = reader.readU16();
        return animation;
    }
};


struct TransitionCase;


struct Transitions
{
    uint16_t stateId;
    uint16_t transitionCaseCount; // number of ranges (seems to always be 1..5)
    uint16_t firstTransitionCase; // Offset into AnimDispatches[]

    gsl::span<const TransitionCase> transitionCases{};

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


struct TransitionCase
{
    uint16_t firstFrame; // Lowest frame that uses this range
    uint16_t lastFrame; // Highest frame (+1?) that uses this range
    uint16_t targetAnimationIndex; // Animation to dispatch to
    uint16_t targetFrame; // Frame offset to dispatch to

    const Animation* targetAnimation = nullptr;

    static std::unique_ptr<TransitionCase> read(io::SDLReader& reader)
    {
        std::unique_ptr<TransitionCase> transition{new TransitionCase()};
        transition->firstFrame = reader.readU16();
        transition->lastFrame = reader.readU16();
        transition->targetAnimationIndex = reader.readU16();
        transition->targetFrame = reader.readU16();
        return transition;
    }
};


struct Mesh;

#pragma pack(push, 1)


struct BoneTreeEntry
{
    uint32_t flags;

    int32_t x, y, z;

    glm::vec3 toGl() const noexcept
    {
        return core::TRVec( x, y, z ).toRenderSystem();
    }
};


#pragma pack(pop)

static_assert( sizeof( BoneTreeEntry ) == 16, "BoneTreeEntry must be of size 16" );


struct SkeletalModelType
{
    engine::TR1ItemId typeId; // Item Identifier (matched in Items[])
    int16_t nmeshes; // number of meshes in this object, or (in case of sprite sequences) the negative number of sprites in the sequence
    uint16_t mesh_base_index; // starting mesh (offset into MeshPointers[])
    uint32_t bone_index; // offset into MeshTree[]
    uint32_t pose_data_offset; // byte offset into Frames[] (divide by 2 for Frames[i])
    uint16_t animation_index; // offset into Animations[]

    gsl::span<gsl::not_null<const loader::Mesh*>> meshes{};
    gsl::span<gsl::not_null<std::shared_ptr<gameplay::Model>>> models{};
    gsl::span<const BoneTreeEntry> boneTree{};

    const AnimFrame* frames = nullptr;

    const Animation* animation = nullptr;

    static std::unique_ptr<SkeletalModelType> readTr1(io::SDLReader& reader)
    {
        std::unique_ptr<SkeletalModelType> moveable{new SkeletalModelType()};
        moveable->typeId = static_cast<engine::TR1ItemId>(reader.readU32());
        moveable->nmeshes = reader.readI16();
        moveable->mesh_base_index = reader.readU16();
        moveable->bone_index = reader.readU32();
        moveable->pose_data_offset = reader.readU32();
        moveable->animation_index = reader.readU16();
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
