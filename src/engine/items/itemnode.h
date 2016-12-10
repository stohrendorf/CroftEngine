#pragma once

#include "audio/sourcehandle.h"
#include "core/interpolatedvalue.h"
#include "engine/skeletalmodelnode.h"

#include <chrono>
#include <set>


namespace loader
{
    struct Item;
}


namespace level
{
    class Level;
}


namespace engine
{
    class LaraNode;


    struct CollisionInfo;


    namespace items
    {
        struct InteractionLimits
        {
            gameplay::BoundingBox distance;
            core::TRRotation minAngle;
            core::TRRotation maxAngle;

            InteractionLimits(const gameplay::BoundingBox& bbox, const core::TRRotation& min, const core::TRRotation& max)
                : distance{bbox}
                , minAngle{min}
                , maxAngle{max}
            {
                distance.repair();
            }

            bool canInteract(const ItemNode& item, const LaraNode& lara) const;
        };


        class ItemNode : public SkeletalModelNode
        {
            core::RoomBoundPosition m_position;

            // needed for YPR rotation, because the scene node uses XYZ rotation
            core::TRRotation m_rotation;

            gsl::not_null<level::Level*> const m_level;

            core::InterpolatedValue<float> m_fallSpeed{0.0f};
            core::InterpolatedValue<float> m_horizontalSpeed{0.0f};

            bool m_falling = false; // flags2_08

            long m_floorHeight = 0;

            std::set<std::weak_ptr<audio::SourceHandle>, audio::WeakSourceHandleLessComparator> m_sounds;

            void updateSounds();

        public:
            static constexpr const uint16_t Oneshot = 0x100;
            static constexpr const uint16_t ActivationMask = 0x3e00;
            static constexpr const uint16_t InvertedActivation = 0x4000;
            static constexpr const uint16_t Locked = 0x8000;

            uint16_t m_itemFlags;
            bool m_isActive = false;
            bool m_flags2_02_toggledOn = false;
            bool m_flags2_04_ready = false;
            bool m_flags2_10 = false;
            bool m_flags2_20 = true;
            bool m_flags2_40 = false;
            bool m_flags2_80 = false;
            std::chrono::microseconds m_triggerTimeout{0};

            const bool m_hasProcessAnimCommandsOverride;
            const uint8_t m_characteristics;


            enum class AnimCommandOpcode : uint16_t
            {
                SetPosition = 1,
                SetVelocity = 2,
                EmptyHands = 3,
                Kill = 4,
                PlaySound = 5,
                PlayEffect = 6,
                Interact = 7
            };


            ItemNode(const gsl::not_null<level::Level*>& level,
                     const std::string& name,
                     const gsl::not_null<const loader::Room*>& room,
                     gsl::not_null<loader::Item*> item,
                     bool hasProcessAnimCommandsOverride,
                     uint8_t characteristics,
                     const loader::AnimatedModel& animatedModel);

            virtual ~ItemNode() = default;


            const core::ExactTRCoordinates& getPosition() const noexcept
            {
                return m_position.position;
            }


            const core::TRRotation& getRotation() const noexcept
            {
                return m_rotation;
            }


            gsl::not_null<const loader::Room*> getCurrentRoom() const noexcept
            {
                return m_position.room;
            }


            long getFloorHeight() const noexcept
            {
                return m_floorHeight;
            }


            void setFloorHeight(long h) noexcept
            {
                m_floorHeight = h;
            }


            void setCurrentRoom(const loader::Room* newRoom);

            void applyTransform();


            void rotate(core::Angle dx, core::Angle dy, core::Angle dz)
            {
                m_rotation.X += dx;
                m_rotation.Y += dy;
                m_rotation.Z += dz;
            }


            void move(float dx, float dy, float dz)
            {
                m_position.position.X += dx;
                m_position.position.Y += dy;
                m_position.position.Z += dz;
            }


            void move(const glm::vec3& d)
            {
                m_position.position += core::ExactTRCoordinates(d);
            }


            void moveLocal(float dx, float dy, float dz)
            {
                const auto sin = getRotation().Y.sin();
                const auto cos = getRotation().Y.cos();
                m_position.position.X += dz * sin + dx * cos;
                m_position.position.Y += dy;
                m_position.position.Z += dz * cos - dx * sin;
            }


            void setPosition(const core::ExactTRCoordinates& pos)
            {
                m_position.position = pos;
            }


            void setXRotation(core::Angle x)
            {
                m_rotation.X = x;
            }


            void addXRotation(core::Angle x)
            {
                m_rotation.X += x;
            }


            void setYRotation(core::Angle y)
            {
                m_rotation.Y = y;
            }


            void addYRotation(core::Angle v)
            {
                m_rotation.Y += v;
            }


            void setZRotation(core::Angle z)
            {
                m_rotation.Z = z;
            }


            void addZRotation(core::Angle z)
            {
                m_rotation.Z += z;
            }


            void setRotation(const core::TRRotation& a)
            {
                m_rotation = a;
            }


            const level::Level& getLevel() const
            {
                return *m_level;
            }


            level::Level& getLevel()
            {
                return *m_level;
            }


            const core::RoomBoundPosition& getRoomBoundPosition() const noexcept
            {
                return m_position;
            }


            bool isFalling() const noexcept
            {
                return m_falling;
            }


            void setFalling(bool falling) noexcept
            {
                m_falling = falling;
            }


            void setFallSpeed(const core::InterpolatedValue<float>& spd)
            {
                m_fallSpeed = spd;
            }


            const core::InterpolatedValue<float>& getFallSpeed() const noexcept
            {
                return m_fallSpeed;
            }


            void setHorizontalSpeed(const core::InterpolatedValue<float>& speed)
            {
                m_horizontalSpeed = speed;
            }


            const core::InterpolatedValue<float>& getHorizontalSpeed() const
            {
                return m_horizontalSpeed;
            }


            void dampenHorizontalSpeed(const std::chrono::microseconds& deltaTime, float f)
            {
                m_horizontalSpeed.sub( m_horizontalSpeed * f, deltaTime );
            }


            virtual void patchFloor(const core::TRCoordinates& /*pos*/, long& /*y*/)
            {
            }


            virtual void patchCeiling(const core::TRCoordinates& /*pos*/, long& /*y*/)
            {
            }


            virtual void onInteract(LaraNode& /*lara*/)
            {
                //BOOST_LOG_TRIVIAL(warning) << "Interaction not implemented: " << m_name;
            }


            void onFrameChanged(FrameChangeType frameChangeType) override;

            void activate();

            void deactivate();

            void update(const std::chrono::microseconds& deltaTime);

            virtual void updateImpl(const std::chrono::microseconds& deltaTime) = 0;


            core::InterpolatedValue<float>& getHorizontalSpeed()
            {
                return m_horizontalSpeed;
            }


            core::InterpolatedValue<float>& getFallSpeed() noexcept
            {
                return m_fallSpeed;
            }


            bool triggerSwitch(uint16_t arg)
            {
                if( !m_flags2_04_ready || m_flags2_02_toggledOn )
                {
                    return false;
                }

                m_flags2_04_ready = false;

                if( getCurrentState() != 0 || loader::isLastFloordataEntry( arg ) )
                {
                    deactivate();
                    m_flags2_02_toggledOn = false;
                }
                else
                {
                    m_triggerTimeout = std::chrono::milliseconds( gsl::narrow_cast<uint8_t>( arg ) );
                    if( m_triggerTimeout.count() != 1 )
                        m_triggerTimeout *= 1000;
                    m_flags2_02_toggledOn = true;
                }

                return true;
            }


            std::shared_ptr<audio::SourceHandle> playSoundEffect(int id);


            bool triggerPickUp()
            {
                if( !m_flags2_04_ready || !m_flags2_02_toggledOn )
                    return false;

                m_flags2_02_toggledOn = false;
                m_flags2_04_ready = true;
                return true;
            }


            bool triggerKey();

            virtual core::Angle getMovementAngle() const
            {
                return getRotation().Y;
            }

        protected:
            bool isInvertedActivation() const noexcept
            {
                return (m_itemFlags & InvertedActivation) != 0;
            }


            bool updateTriggerTimeout(const std::chrono::microseconds& deltaTime)
            {
                if( (m_itemFlags & ActivationMask) != ActivationMask )
                {
                    return isInvertedActivation();
                }

                if( m_triggerTimeout == std::chrono::microseconds::zero() )
                {
                    return !isInvertedActivation();
                }

                if( m_triggerTimeout < std::chrono::microseconds::zero() )
                {
                    return isInvertedActivation();
                }

                BOOST_ASSERT( deltaTime > std::chrono::microseconds::zero() );
                m_triggerTimeout -= deltaTime;
                if( m_triggerTimeout <= std::chrono::microseconds::zero() )
                    m_triggerTimeout = std::chrono::microseconds( -1 );

                return !isInvertedActivation();
            }
        };
    }
}
