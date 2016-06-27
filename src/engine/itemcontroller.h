#pragma once

#include "core/angle.h"
#include "core/coordinates.h"
#include "level/level.h"
#include "core/interpolatedvalue.h"

#include <irrlicht.h>

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
    class AnimationController;
    struct LaraState;

    struct InteractionLimits
    {
        irr::core::aabbox3di distance;
        irr::core::vector3d<core::Angle> minAngle;
        irr::core::vector3d<core::Angle> maxAngle;

        bool canInteract(const ItemController& item, const LaraController& lara) const;
    };

    class ItemController : public irr::scene::ISceneNodeAnimator
    {
        core::RoomBoundPosition m_position;

        // needed for YPR rotation, because the scene node uses XYZ rotation
        irr::core::vector3d<core::Angle> m_rotation;

        gsl::not_null<level::Level*> const m_level;

        gsl::not_null<irr::scene::ISceneNode*> const m_sceneNode;

        std::shared_ptr<engine::AnimationController> m_dispatcher;
        const std::string m_name;

        int m_lastAnimFrame = -1;
        core::InterpolatedValue<float> m_fallSpeed{ 0.0f };
        core::InterpolatedValue<float> m_horizontalSpeed{ 0.0f };
        int m_currentFrameTime = 0;
        int m_lastFrameTime = -1;
        int m_lastEngineFrameTime = -1;

        bool m_falling = false; // flags2_08

    public:
        uint16_t m_itemFlags;
        bool m_isActive = false;
        bool m_flags2_02 = false;
        bool m_flags2_04 = false;
        bool m_flags2_10 = false;
        bool m_flags2_20 = true;
        bool m_flags2_40 = false;
        bool m_flags2_80 = false;
        uint16_t m_triggerFlags = 0;
        int m_triggerTimeout = 0;

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

        static constexpr int FrameTime = 1000 / 30;

        ItemController(const gsl::not_null<level::Level*>& level,
                       const std::shared_ptr<engine::AnimationController>& dispatcher,
                       const gsl::not_null<irr::scene::ISceneNode*>& sceneNode,
                       const std::string& name,
                       const gsl::not_null<const loader::Room*>& room,
                       gsl::not_null<loader::Item*> item,
                       bool hasProcessAnimCommandsOverride,
                       uint8_t characteristics);

        irr::core::aabbox3di getBoundingBox() const;

        const core::ExactTRCoordinates& getPosition() const noexcept
        {
            return m_position.position;
        }

        const irr::core::vector3d<core::Angle>& getRotation() const noexcept
        {
            return m_rotation;
        }

        gsl::not_null<const loader::Room*> getCurrentRoom() const noexcept
        {
            return m_position.room;
        }

        void setCurrentRoom(const loader::Room* newRoom);

        void applyRotation();

        void applyPosition()
        {
            if(auto parent = m_sceneNode->getParent())
                m_sceneNode->setPosition(m_position.position.toIrrlicht() - parent->getAbsolutePosition());
            else
                m_sceneNode->setPosition(m_position.position.toIrrlicht());

            m_sceneNode->updateAbsolutePosition();
        }

        irr::scene::ISceneNode* getSceneNode() const noexcept
        {
            return m_sceneNode;
        }

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

        const level::Level& getLevel() const
        {
            return *m_level;
        }

        level::Level& getLevel()
        {
            return *m_level;
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

        void dampenHorizontalSpeed(float f)
        {
            m_horizontalSpeed.sub(m_horizontalSpeed * f, getCurrentDeltaTime());
        }

        int getCurrentDeltaTime() const
        {
            return m_currentFrameTime - m_lastFrameTime;
        }

        void setTargetState(uint16_t st);
        uint16_t getTargetState() const;

        void playAnimation(uint16_t anim, const boost::optional<irr::u32>& firstFrame);

        void nextFrame();
        irr::u32 getCurrentFrame() const;
        irr::u32 getAnimEndFrame() const;
        uint16_t getCurrentAnimState() const;
        uint16_t getCurrentAnimationId() const;

        bool handleTRTransitions();

        float calculateAnimFloorSpeed() const;
        int getAnimAccelleration() const;

        virtual float queryFloor(const core::ExactTRCoordinates& pos)
        {
            return pos.Y;
        }

        virtual float queryCeiling(const core::ExactTRCoordinates& pos)
        {
            return pos.Y;
        }

        virtual void onInteract(LaraController& lara, LaraState& state)
        {
            BOOST_LOG_TRIVIAL(warning) << "Interaction not implemented: " << m_name;
        }

        virtual void processAnimCommands(bool advanceFrame = false);
        
        void activate();
        void deactivate();

        void animateNode(irr::scene::ISceneNode* node, irr::u32 timeMs) override final
        {
            Expects(node == m_sceneNode);

            if(m_lastFrameTime < 0)
                m_lastFrameTime = m_lastEngineFrameTime = m_currentFrameTime = timeMs;

            if(m_lastFrameTime == timeMs)
                return;

            m_currentFrameTime = timeMs;

            bool isNewFrame = m_dispatcher == nullptr ? false : m_lastAnimFrame != getCurrentFrame();

            if(timeMs - m_lastEngineFrameTime >= FrameTime)
            {
                isNewFrame = true;
                m_lastEngineFrameTime -= (timeMs - m_lastEngineFrameTime) / FrameTime * FrameTime;
            }

            animate(isNewFrame);

            applyRotation();
            applyPosition();

            m_lastFrameTime = m_currentFrameTime;
        }

        ISceneNodeAnimator* createClone(irr::scene::ISceneNode* node, irr::scene::ISceneManager* newManager) override final
        {
            BOOST_ASSERT(false);
            return nullptr;
        }

        virtual void animate(bool isNewFrame) = 0;

        core::InterpolatedValue<float>& getHorizontalSpeed()
        {
            return m_horizontalSpeed;
        }

        core::InterpolatedValue<float>& getFallSpeed() noexcept
        {
            return m_fallSpeed;
        }

    protected:
        int getLastAnimFrame() const noexcept
        {
            return m_lastAnimFrame;
        }

        void setLastAnimFrame(int f) noexcept
        {
            m_lastAnimFrame = f;
        }

        bool updateTrigger()
        {
            if((m_itemFlags&0x3e00) != 0x3e00)
            {
                return (m_itemFlags & 0x40) != 0;
            }

            if(m_triggerTimeout == 0)
            {
                return (m_itemFlags & 0x40) == 0;
            }
            
            if(m_triggerTimeout < 0)
            {
                return (m_itemFlags & 0x40) != 0;
            }

            m_triggerTimeout -= getCurrentDeltaTime();
            if(m_triggerTimeout < 0)
                m_triggerTimeout = -1;
            return (m_itemFlags & 0x40) == 0;
        }
    };

    class DummyItemController final : public ItemController
    {
    public:
        DummyItemController(const gsl::not_null<level::Level*>& level, const std::shared_ptr<engine::AnimationController>& dispatcher, const gsl::not_null<irr::scene::ISceneNode*>& sceneNode, const std::string& name, const gsl::not_null<const loader::Room*>& room, const gsl::not_null<loader::Item*>& item)
            : ItemController(level, dispatcher, sceneNode, name, room, item, false, 0)
        {
        }

        void animate(bool /*isNewFrame*/) override
        {
        }
    };

    class ItemController_55_Switch final : public ItemController
    {
    public:
        ItemController_55_Switch(const gsl::not_null<level::Level*>& level, const std::shared_ptr<engine::AnimationController>& dispatcher, const gsl::not_null<irr::scene::ISceneNode*>& sceneNode, const std::string& name, const gsl::not_null<const loader::Room*>& room, const gsl::not_null<loader::Item*>& item)
            : ItemController(level, dispatcher, sceneNode, name, room, item, true, 0x30)
        {
        }

        void animate(bool /*isNewFrame*/) override
        {
        }

        void onInteract(LaraController& lara, LaraState& state) override;
      
        void processAnimCommands(bool advanceFrame = false) override
        {
            m_triggerFlags |= 0x3e00;
            if(!updateTrigger())
            {
                setTargetState(1);
                m_triggerTimeout = 0;
            }

            ItemController::processAnimCommands(advanceFrame);
        }
    };

    class ItemController_62_Door final : public ItemController
    {
    public:
        ItemController_62_Door(const gsl::not_null<level::Level*>& level, const std::shared_ptr<engine::AnimationController>& dispatcher, const gsl::not_null<irr::scene::ISceneNode*>& sceneNode, const std::string& name, const gsl::not_null<const loader::Room*>& room, const gsl::not_null<loader::Item*>& item)
            : ItemController(level, dispatcher, sceneNode, name, room, item, true, 0x30)
        {
        }

        void animate(bool /*isNewFrame*/) override
        {
        }

        void onInteract(LaraController& lara, LaraState& state) override;
      
        void processAnimCommands(bool advanceFrame = false) override
        {
            BOOST_LOG_TRIVIAL(warning) << "Door anim command processing not fully implemented";

            if(updateTrigger())
            {
                if(getCurrentAnimState() != 0)
                {
                    //! @todo Implement me
                }
                else
                {
                    setTargetState(1);
                    ItemController::processAnimCommands();
                    return;
                }
            }
            else
            {
                if(getCurrentAnimState() == 1)
                {
                    setTargetState(0);
                    ItemController::processAnimCommands();
                    return;
                }
            }
            ItemController::processAnimCommands();
        }
    };
}
