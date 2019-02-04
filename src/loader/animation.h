#pragma once

#include "io/sdlreader.h"
#include "engine/items_tr1.h"
#include "core/vec.h"
#include "core/frame.h"
#include "gameplay.h"

#include "gsl-lite.hpp"
#include "type_safe/types.hpp"

namespace loader
{
#pragma pack(push, 1)


struct BoundingBox
{
    core::Length minX{0_len}, maxX{0_len};
    core::Length minY{0_len}, maxY{0_len};
    core::Length minZ{0_len}, maxZ{0_len};

    explicit BoundingBox() = default;

    BoundingBox(
            core::Length minX,
            core::Length maxX,
            core::Length minY,
            core::Length maxY,
            core::Length minZ,
            core::Length maxZ
    )
            : minX{minX}, maxX{maxX}
            , minY{minY}, maxY{maxY}
            , minZ{minZ}, maxZ{maxZ}
    {}

    BoundingBox(const BoundingBox& a, const BoundingBox& b, const float bias)
            : minX{lerp( a.minX, b.minX, bias )}
            , maxX{lerp( a.maxX, b.maxX, bias )}
            , minY{lerp( a.minY, b.minY, bias )}
            , maxY{lerp( a.maxY, b.maxY, bias )}
            , minZ{lerp( a.minZ, b.minZ, bias )}
            , maxZ{lerp( a.maxZ, b.maxZ, bias )}
    {
    }

    core::TRVec getCenter() const
    {
        return {(minX + maxX) / 2, (minY + maxY) / 2, (minZ + maxZ) / 2};
    }
};


struct BoundingBoxIO
{
    int16_t minX{0}, maxX{0};
    int16_t minY{0}, maxY{0};
    int16_t minZ{0}, maxZ{0};

    BoundingBox toBBox() const
    {
        return BoundingBox{
                core::Length{static_cast<core::Length::int_type>(minX)},
                core::Length{static_cast<core::Length::int_type>(maxX)},
                core::Length{static_cast<core::Length::int_type>(minY)},
                core::Length{static_cast<core::Length::int_type>(maxY)},
                core::Length{static_cast<core::Length::int_type>(minZ)},
                core::Length{static_cast<core::Length::int_type>(maxZ)}
        };
    }
};


struct AnimFrame
{
    struct Vec
    {
        int16_t x, y, z;

        glm::vec3 toGl() const noexcept
        {
            return toTr().toRenderSystem();
        }

        core::TRVec toTr() const noexcept
        {
            return core::TRVec{
                    core::Length{static_cast<core::Length::int_type>(x)},
                    core::Length{static_cast<core::Length::int_type>(y)},
                    core::Length{static_cast<core::Length::int_type>(z)}
            };
        }
    };


    BoundingBoxIO bbox;
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

using AnimState = type_safe::integer<uint16_t>;

template<char... Digits>
inline constexpr AnimState operator "" _as()
{
    return type_safe::operator ""_u16 < Digits...>();
}


struct Animation
{
    uint32_t poseDataOffset; // byte offset into Frames[] (divide by 2 for Frames[i])

    const AnimFrame* frames = nullptr;

    core::Frame segmentLength = 0_frame; // Slowdown factor of this animation
    uint8_t poseDataSize; // number of bit16's in Frames[] used by this animation
    AnimState state_id = 0_as;

    int32_t speed;
    int32_t acceleration;

    int32_t lateralSpeed; // new in TR4 -->
    int32_t lateralAcceleration; // lateral speed and acceleration.

    core::Frame firstFrame = 0_frame; // first frame in this animation
    core::Frame lastFrame = 0_frame; // last frame in this animation (numframes = (End - Start) + 1)
    uint16_t nextAnimationIndex;
    core::Frame nextFrame = 0_frame;

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

    constexpr core::Frame getFrameCount() const
    {
        return lastFrame - firstFrame + 1_frame;
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
    static std::unique_ptr<Animation> read(io::SDLReader& reader, const bool withLateral)
    {
        auto animation = std::make_unique<Animation>();
        animation->poseDataOffset = reader.readU32();
        animation->segmentLength = core::Frame{static_cast<core::Frame::int_type>(reader.readU8())};
        if( animation->segmentLength == 0_frame )
            animation->segmentLength = 1_frame;
        animation->poseDataSize = reader.readU8();
        animation->state_id = AnimState{reader.readU16()};

        animation->speed = reader.readI32();
        animation->acceleration = reader.readI32();
        if( withLateral )
        {
            animation->lateralSpeed = reader.readI32();
            animation->lateralAcceleration = reader.readI32();
        }
        else
        {
            animation->lateralSpeed = 0;
            animation->lateralAcceleration = 0;
        }

        animation->firstFrame = core::Frame{static_cast<core::Frame::int_type>(reader.readU16())};
        animation->lastFrame = core::Frame{static_cast<core::Frame::int_type>(reader.readU16())};
        animation->nextAnimationIndex = reader.readU16();
        animation->nextFrame = core::Frame{static_cast<core::Frame::int_type>(reader.readU16())};

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
        std::unique_ptr<Transitions> state_change = std::make_unique<Transitions>();
        state_change->stateId = reader.readU16();
        state_change->transitionCaseCount = reader.readU16();
        state_change->firstTransitionCase = reader.readU16();
        return state_change;
    }
};


struct TransitionCase
{
    core::Frame firstFrame = 0_frame; // Lowest frame that uses this range
    core::Frame lastFrame = 0_frame; // Highest frame (+1?) that uses this range
    uint16_t targetAnimationIndex; // Animation to dispatch to
    core::Frame targetFrame = 0_frame; // Frame offset to dispatch to

    const Animation* targetAnimation = nullptr;

    static std::unique_ptr<TransitionCase> read(io::SDLReader& reader)
    {
        std::unique_ptr<TransitionCase> transition{new TransitionCase()};
        transition->firstFrame = core::Frame{static_cast<core::Frame::int_type>(reader.readU16())};
        transition->lastFrame = core::Frame{static_cast<core::Frame::int_type>(reader.readU16())};
        transition->targetAnimationIndex = reader.readU16();
        transition->targetFrame = core::Frame{static_cast<core::Frame::int_type>(reader.readU16())};
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
        return core::TRVec( core::Length{x}, core::Length{y}, core::Length{z} ).toRenderSystem();
    }
};


#pragma pack(pop)

static_assert( sizeof( BoneTreeEntry ) == 16, "BoneTreeEntry must be of size 16" );


struct SkeletalModelType
{
    engine::TR1ItemId type;
    int16_t nMeshes; // number of meshes in this object, or (in case of sprite sequences) the negative number of sprites in the sequence
    uint16_t mesh_base_index; // starting mesh (offset into MeshPointers[])
    uint32_t bone_index; // offset into MeshTree[]
    uint32_t pose_data_offset; // byte offset into Frames[] (divide by 2 for Frames[i])
    uint16_t animation_index; // offset into Animations[]

    gsl::span<gsl::not_null<const Mesh*>> meshes{};
    gsl::span<gsl::not_null<std::shared_ptr<gameplay::Model>>> models{};
    gsl::span<const BoneTreeEntry> boneTree{};

    const AnimFrame* frames = nullptr;

    const Animation* animations = nullptr;

    static std::unique_ptr<SkeletalModelType> readTr1(io::SDLReader& reader)
    {
        std::unique_ptr<SkeletalModelType> moveable{new SkeletalModelType()};
        moveable->type = static_cast<engine::TR1ItemId>(reader.readU32());
        moveable->nMeshes = reader.readI16();
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

using loader::operator ""_as;
