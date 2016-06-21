#pragma once

#include "datatypes.h"

class ItemController : public irr::scene::ISceneNodeAnimator
{
    loader::RoomBoundPosition m_position;

    // needed for YPR rotation, because the scene node uses XYZ rotation
    irr::core::vector3d<core::Angle> m_rotation;

    gsl::not_null<const loader::Level*> const m_level;

    gsl::not_null<irr::scene::ISceneNode*> const m_sceneNode;

    std::shared_ptr<loader::AnimationController> m_dispatcher;
    const std::string m_name;

public:
    ItemController(const gsl::not_null<const loader::Level*>& level, const std::shared_ptr<loader::AnimationController>& dispatcher, const gsl::not_null<irr::scene::ISceneNode*>& sceneNode, const std::string& name, const gsl::not_null<const loader::Room*>& room)
        : m_level(level)
        , m_dispatcher(dispatcher)
        , m_name(name)
        , m_sceneNode(sceneNode)
        , m_position(room)
    {
        Expects(dispatcher != nullptr);

        auto laraRot = sceneNode->getRotation();
        m_rotation.X = core::Angle::fromDegrees(laraRot.X);
        m_rotation.Y = core::Angle::fromDegrees(laraRot.Y);
        m_rotation.Z = core::Angle::fromDegrees(laraRot.Z);

        m_sceneNode->updateAbsolutePosition();
        m_position.position = loader::ExactTRCoordinates(m_sceneNode->getAbsolutePosition());

        setCurrentRoom(room);
    }

    irr::core::aabbox3di getBoundingBox() const;

    const loader::ExactTRCoordinates& getPosition() const noexcept
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

    void setPosition(const loader::ExactTRCoordinates& pos)
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

    const loader::Level& getLevel() const
    {
        return *m_level;
    }

    void setTargetState(uint16_t st);
    uint16_t getTargetState() const;

    void playAnimation(uint16_t anim, const boost::optional<irr::u32>& firstFrame);

    irr::u32 getCurrentFrame() const;
    irr::u32 getAnimEndFrame() const;
    uint16_t getCurrentAnimState() const;
    uint16_t getCurrentAnimationId() const;

    bool handleTRTransitions();

    float calculateAnimFloorSpeed() const;
    int getAnimAccelleration() const;
};
