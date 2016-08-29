#pragma once

#include "io/sdlreader.h"

#include <gsl.h>

#include <map>

namespace loader
{
    /** \brief animation->
    *
    * This describes each individual animation; these may be looped by specifying
    * the next animation to be itself. In TR2 and TR3, one must be careful when
    * parsing frames using the FrameSize value as the size of each frame, since
    * an animation's frame range may extend into the next animation's frame range,
    * and that may have a different FrameSize value.
    */
    struct Animation
    {
        uint32_t poseDataOffset; // byte offset into Frames[] (divide by 2 for Frames[i])
        uint8_t stretchFactor; // Slowdown factor of this animation
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
            return (lastFrame - firstFrame + stretchFactor) / stretchFactor;
        }

        constexpr size_t getFrameCount() const
        {
            return lastFrame - firstFrame + 1;
        }

        /// \brief reads an animation definition.
        static std::unique_ptr<Animation> readTr1(io::SDLReader& reader)
        {
            return read(reader, false);
        }

        static std::unique_ptr<Animation> readTr4(io::SDLReader& reader)
        {
            return read(reader, true);
        }

    private:
        static std::unique_ptr<Animation> read(io::SDLReader& reader, bool withLateral)
        {
            std::unique_ptr<Animation> animation{new Animation()};
            animation->poseDataOffset = reader.readU32();
            animation->stretchFactor = reader.readU8();
            if( animation->stretchFactor == 0 )
                animation->stretchFactor = 1;
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

    struct AnimatedModel
    {
        uint32_t type; // Item Identifier (matched in Items[])
        uint16_t meshCount; // number of meshes in this object
        uint16_t firstMesh; // starting mesh (offset into MeshPointers[])
        uint32_t boneTreeIndex; // offset into MeshTree[]
        uint32_t meshPositionOffset; // byte offset into Frames[] (divide by 2 for Frames[i])
        uint16_t animationIndex; // offset into Animations[]

        /**
        * @brief Describes a range of frames of the linearized animations in an Irrlicht IAnimatedMeshSceneNode
        */
        struct FrameRange
        {
            //! The first real frame in the linearized animation this range describes
            const uint32_t offset;
            //! The first frame of the source animation frame range
            const uint32_t firstFrame;
            //! The last frame of the source animation frame range
            const uint32_t lastFrame;
            const std::map<uint32_t, gameplay::BoundingBox> bboxes;
            const gsl::not_null<gameplay::AnimationClip*> animation;

            FrameRange(uint32_t o, uint32_t f, uint32_t l, std::map<uint32_t, gameplay::BoundingBox>&& bb, const gsl::not_null<gameplay::AnimationClip*>& anim)
                : offset(o)
                , firstFrame(f)
                , lastFrame(l)
                , bboxes(std::move(bb))
                , animation(anim)
            {
                BOOST_ASSERT(firstFrame <= lastFrame);
                BOOST_ASSERT(!bboxes.empty());
            }

            void apply(gameplay::AnimationController& ctrl, uint32_t localFrame) const
            {
                BOOST_ASSERT(localFrame >= firstFrame && localFrame <= lastFrame);

                const auto realOffset = offset + (localFrame - firstFrame);
                const auto realFirst = offset;
                const auto realLast = offset + lastFrame - firstFrame;

                BOOST_ASSERT(realFirst <= realLast);
                ctrl.stopAllAnimations();
                animation->play(gsl::narrow_cast<double>(realOffset) / core::FrameRate);
            }

            gameplay::BoundingBox getBoundingBox(uint32_t localFrame) const
            {
                BOOST_ASSERT(localFrame >= firstFrame && localFrame <= lastFrame);
                localFrame -= firstFrame;
                auto it = bboxes.lower_bound(localFrame);
                if( it == bboxes.end() )
                    return std::prev(it)->second;

                if( it->first == localFrame || it == bboxes.begin() )
                    return it->second;

                // the iterator points behind the searched frame
                auto before = std::prev(it);
                auto dist = it->first - before->first;
                BOOST_ASSERT(dist > 0);
                auto lambda = float(localFrame - before->first) / dist;

                // aabbox's getInterpolated does wrong rounding for ints, so we need to do it manually
                gameplay::BoundingBox interp(before->second.min.lerp(it->second.min, lambda), before->second.max.lerp(it->second.max, lambda));
                return interp;
            }
        };

        std::map<uint16_t, FrameRange> frameMapping;

        /** \brief reads a moveable definition.
        *
        * some sanity checks get done which throw a exception on failure.
        * frame_offset needs to be corrected later in TR_Level::read_tr_level.
        */
        static std::unique_ptr<AnimatedModel> readTr1(io::SDLReader& reader)
        {
            std::unique_ptr<AnimatedModel> moveable{new AnimatedModel()};
            moveable->type = reader.readU32();
            moveable->meshCount = reader.readU16();
            moveable->firstMesh = reader.readU16();
            moveable->boneTreeIndex = reader.readU32();
            moveable->meshPositionOffset = reader.readU32();
            moveable->animationIndex = reader.readU16();
            return moveable;
        }

        static std::unique_ptr<AnimatedModel> readTr5(io::SDLReader& reader)
        {
            std::unique_ptr<AnimatedModel> moveable = readTr1(reader);
            if( reader.readU16() != 0xFFEF )
            BOOST_LOG_TRIVIAL(warning) << "TR5 Moveable: filler has wrong value";
            return moveable;
        }
    };
}
