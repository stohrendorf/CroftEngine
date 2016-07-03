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
    class MeshAnimationController;
    struct LaraState;

    struct InteractionLimits
    {
        irr::core::aabbox3di distance;
        irr::core::vector3d<core::Angle> minAngle;
        irr::core::vector3d<core::Angle> maxAngle;

        bool canInteract(const ItemController& item, const LaraController& lara) const;
    };

    class ItemController
    {
        core::RoomBoundPosition m_position;

        // needed for YPR rotation, because the scene node uses XYZ rotation
        irr::core::vector3d<core::Angle> m_rotation;

        gsl::not_null<level::Level*> const m_level;

        gsl::not_null<irr::scene::ISceneNode*> const m_sceneNode;

        std::shared_ptr<engine::MeshAnimationController> m_dispatcher;
        const std::string m_name;

        int m_lastAnimFrame = -1;
        core::InterpolatedValue<float> m_fallSpeed{ 0.0f };
        core::InterpolatedValue<float> m_horizontalSpeed{ 0.0f };
        uint32_t m_currentDeltaTime = 0;
        uint32_t m_subFrameTime = 0;

        bool m_falling = false; // flags2_08

        int m_floorHeight = 0;

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
                       const std::shared_ptr<engine::MeshAnimationController>& dispatcher,
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

        int getFloorHeight() const noexcept
        {
            return m_floorHeight;
        }

        void setFloorHeight(int h) noexcept
        {
            m_floorHeight = h;
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

        void dampenHorizontalSpeed(float f)
        {
            m_horizontalSpeed.sub(m_horizontalSpeed * f, getCurrentDeltaTime());
        }

        uint32_t getCurrentDeltaTime() const
        {
            return m_currentDeltaTime;
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
        void handleAnimationEnd();

        float calculateAnimFloorSpeed() const;
        int getAnimAccelleration() const;

        virtual void patchFloor(const core::TRCoordinates& pos, int& y)
        {
        }

        virtual void patchCeiling(const core::TRCoordinates& pos, int& y)
        {
        }

        virtual void onInteract(LaraController& lara, LaraState& state)
        {
            //BOOST_LOG_TRIVIAL(warning) << "Interaction not implemented: " << m_name;
        }

        virtual void processAnimCommands(bool advanceFrame = false);

        void activate();
        void deactivate();

        void update(uint32_t deltaTimeMs)
        {
            m_currentDeltaTime = deltaTimeMs;

            if(m_currentDeltaTime <= 0)
                return;

            bool isNewFrame = m_dispatcher == nullptr ? false : m_lastAnimFrame != getCurrentFrame();
            m_subFrameTime += deltaTimeMs;

            if(m_subFrameTime >= FrameTime)
            {
                isNewFrame = true;
                m_subFrameTime -= m_subFrameTime / FrameTime * FrameTime;
            }

            animateImpl(isNewFrame);

            if(isNewFrame && m_isActive && m_hasProcessAnimCommandsOverride)
                processAnimCommands();

            applyRotation();
            applyPosition();
        }

        virtual void animateImpl(bool isNewFrame) = 0;

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
            if(!m_flags2_04_ready || m_flags2_02_toggledOn)
            {
                return false;
            }

            m_flags2_04_ready = false;

            if(getCurrentAnimState() != 0 || loader::isLastFloordataEntry(arg))
            {
                deactivate();
                m_flags2_02_toggledOn = false;
            }
            else
            {
                m_triggerTimeout = gsl::narrow_cast<uint8_t>(arg);
                if(m_triggerTimeout != 1)
                    m_triggerTimeout *= 1000;
                m_flags2_02_toggledOn = true;
            }

            return true;
        }

        const std::string& getName() const noexcept
        {
            return m_name;
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

        bool isInvertedActivation() const noexcept
        {
            return (m_itemFlags & InvertedActivation) != 0;
        }

        bool updateTriggerTimeout()
        {
            if((m_itemFlags&ActivationMask) != ActivationMask)
            {
                return isInvertedActivation();
            }

            if(m_triggerTimeout == 0)
            {
                return !isInvertedActivation();
            }

            if(m_triggerTimeout < 0)
            {
                return isInvertedActivation();
            }

            BOOST_ASSERT(getCurrentDeltaTime() > 0);
            m_triggerTimeout -= getCurrentDeltaTime();
            if(m_triggerTimeout <= 0)
                m_triggerTimeout = -1;

            return !isInvertedActivation();
        }
    };

    class DummyItemController final : public ItemController
    {
    public:
        DummyItemController(const gsl::not_null<level::Level*>& level, const std::shared_ptr<engine::MeshAnimationController>& dispatcher, const gsl::not_null<irr::scene::ISceneNode*>& sceneNode, const std::string& name, const gsl::not_null<const loader::Room*>& room, const gsl::not_null<loader::Item*>& item)
            : ItemController(level, dispatcher, sceneNode, name, room, item, false, 0)
        {
        }

        void animateImpl(bool /*isNewFrame*/) override
        {
        }
    };

    class ItemController_55_Switch final : public ItemController
    {
    public:
        ItemController_55_Switch(const gsl::not_null<level::Level*>& level, const std::shared_ptr<engine::MeshAnimationController>& dispatcher, const gsl::not_null<irr::scene::ISceneNode*>& sceneNode, const std::string& name, const gsl::not_null<const loader::Room*>& room, const gsl::not_null<loader::Item*>& item)
            : ItemController(level, dispatcher, sceneNode, name, room, item, true, 0x30)
        {
        }

        void animateImpl(bool /*isNewFrame*/) override
        {
        }

        void onInteract(LaraController& lara, LaraState& state) override;

        void processAnimCommands(bool advanceFrame = false) override
        {
            m_itemFlags |= ActivationMask;
            if(!updateTriggerTimeout())
            {
                setTargetState(1);
                m_triggerTimeout = 0;
            }

            ItemController::processAnimCommands();
        }
    };

    class ItemController_35_CollapsibleFloor final : public ItemController
    {
    public:
        ItemController_35_CollapsibleFloor(const gsl::not_null<level::Level*>& level, const std::shared_ptr<engine::MeshAnimationController>& dispatcher, const gsl::not_null<irr::scene::ISceneNode*>& sceneNode, const std::string& name, const gsl::not_null<const loader::Room*>& room, const gsl::not_null<loader::Item*>& item)
            : ItemController(level, dispatcher, sceneNode, name, room, item, true, 0x34)
        {
        }

        void animateImpl(bool /*isNewFrame*/) override
        {
        }

        void onInteract(LaraController& lara, LaraState& state) override
        {
        }

        void processAnimCommands(bool advanceFrame = false) override;

        void patchFloor(const core::TRCoordinates& pos, int& y) override
        {
            if(pos.Y > getPosition().Y - 512)
                return;

            if(getCurrentAnimState() != 0 && getCurrentAnimState() != 1)
                return;

            y = getPosition().Y - 512;
        }

        void patchCeiling(const core::TRCoordinates& pos, int& y) override
        {
            if(pos.Y <= getPosition().Y - 512)
                return;

            if(getCurrentAnimState() != 0 && getCurrentAnimState() != 1)
                return;

            y = getPosition().Y - 256;
        }
    };

    class ItemController_Door final : public ItemController
    {
    public:
        ItemController_Door(const gsl::not_null<level::Level*>& level, const std::shared_ptr<engine::MeshAnimationController>& dispatcher, const gsl::not_null<irr::scene::ISceneNode*>& sceneNode, const std::string& name, const gsl::not_null<const loader::Room*>& room, const gsl::not_null<loader::Item*>& item)
            : ItemController(level, dispatcher, sceneNode, name, room, item, true, 0x30)
        {
        }

        void animateImpl(bool /*isNewFrame*/) override
        {
        }

        void onInteract(LaraController& lara, LaraState& state) override;

        void processAnimCommands(bool /*advanceFrame*/ = false) override
        {
            //BOOST_LOG_TRIVIAL(warning) << "Door anim command processing not fully implemented";

            if(updateTriggerTimeout())
            {
                if(getCurrentAnimState() != 0)
                {
                    //! @todo Restore original sector data
                }
                else
                {
                    BOOST_LOG_TRIVIAL(debug) << "Door " << getName() << ": opening";
                    setTargetState(1);
                }
            }
            else
            {
                if(getCurrentAnimState() == 1)
                {
                    BOOST_LOG_TRIVIAL(debug) << "Door " << getName() << ": closing";
                    setTargetState(0);
                    ItemController::processAnimCommands();
                    return;
                }
                //! @todo Patch original sector data with blocking heights
            }
            ItemController::processAnimCommands();
        }
    };

    class ItemController_Block final : public ItemController
    {
    public:
        ItemController_Block(const gsl::not_null<level::Level*>& level, const std::shared_ptr<engine::MeshAnimationController>& dispatcher, const gsl::not_null<irr::scene::ISceneNode*>& sceneNode, const std::string& name, const gsl::not_null<const loader::Room*>& room, const gsl::not_null<loader::Item*>& item)
            : ItemController(level, dispatcher, sceneNode, name, room, item, true, 0x34)
        {
            if(!m_flags2_04_ready || !m_flags2_02_toggledOn)
                loader::Room::patchHeightsForBlock(*this, -loader::SectorSize);
        }

        void animateImpl(bool /*isNewFrame*/) override
        {
        }

        void onInteract(LaraController& lara, LaraState& state) override;

        void processAnimCommands(bool advanceFrame = false) override;

    private:
        bool isOnFloor(int height) const;

        bool canPushBlock(int height, core::Axis axis) const;
        bool canPullBlock(int height, core::Axis axis) const;
    };

    class ItemController_TallBlock final : public ItemController
    {
    public:
        ItemController_TallBlock(const gsl::not_null<level::Level*>& level, const std::shared_ptr<engine::MeshAnimationController>& dispatcher, const gsl::not_null<irr::scene::ISceneNode*>& sceneNode, const std::string& name, const gsl::not_null<const loader::Room*>& room, const gsl::not_null<loader::Item*>& item)
            : ItemController(level, dispatcher, sceneNode, name, room, item, true, 0x34)
        {
            loader::Room::patchHeightsForBlock(*this, -2*loader::SectorSize);
        }

        void animateImpl(bool /*isNewFrame*/) override
        {
        }

        void onInteract(LaraController& /*lara*/, LaraState& /*state*/) override
        {
        }

        void processAnimCommands(bool advanceFrame = false) override;

    private:
        bool isOnFloor(int height) const;

        bool canPushBlock(int height, core::Axis axis) const;
        bool canPullBlock(int height, core::Axis axis) const;
    };
}
