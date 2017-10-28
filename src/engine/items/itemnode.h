#pragma once

#include "audio/sourcehandle.h"
#include "core/boundingbox.h"
#include "engine/floordata/floordata.h"
#include "engine/skeletalmodelnode.h"

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
            core::BoundingBox distance;
            core::TRRotation minAngle;
            core::TRRotation maxAngle;


            InteractionLimits(const core::BoundingBox& bbox, const core::TRRotation& min, const core::TRRotation& max)
                : distance{ bbox }
                , minAngle{ min }
                , maxAngle{ max }
            {
                distance.makeValid();
            }


            bool canInteract(const ItemNode& item, const LaraNode& lara) const;
        };


        enum class TriggerState
        {
            Disabled,
            Enabled,
            Activated,
            Locked
        };


        class ItemNode
        {
            core::RoomBoundPosition m_position;

            // needed for YPR rotation, because the scene node uses XYZ rotation
            core::TRRotation m_rotation;

            gsl::not_null<level::Level*> const m_level;

            int m_fallSpeed{ 0 };
            int m_horizontalSpeed{ 0 };

            bool m_falling = false; // flags2_08

            int m_floorHeight = 0;

            std::set<std::weak_ptr<audio::SourceHandle>, audio::WeakSourceHandleLessComparator> m_sounds;

            void updateSounds();

        public:
            using Characteristics = uint8_t;
            static const constexpr Characteristics Intelligent = 0x02;
            static const constexpr Characteristics NonLot = 0x04;
            static const constexpr Characteristics SavePosition = 0x08;
            static const constexpr Characteristics SaveHitpoints = 0x10;
            static const constexpr Characteristics SaveFlags = 0x20;
            static const constexpr Characteristics SaveAnim = 0x40;
            static const constexpr Characteristics SemiTransparent = 0x40;

            floordata::ActivationState m_activationState;
            bool m_isActive = false;
            TriggerState m_triggerState = TriggerState::Disabled;
            bool m_flags2_10_isHit = false;
            bool m_flags2_20_collidable = true;
            bool m_flags2_40_alreadyLookedAt = false;
            bool m_flags2_80_dynamicLight = false;

            const bool m_hasProcessAnimCommandsOverride;
            const Characteristics m_characteristics;
            const int16_t m_darkness;

            struct Lighting
            {
                glm::vec3 position;
                float base;
                float baseDiff;
            };


            Lighting m_lighting;


            enum class AnimCommandOpcode : uint16_t
            {
                SetPosition = 1,
                StartFalling = 2,
                EmptyHands = 3,
                Kill = 4,
                PlaySound = 5,
                PlayEffect = 6,
                Interact = 7
            };


            ItemNode(const gsl::not_null<level::Level*>& level,
                const gsl::not_null<const loader::Room*>& room,
                const core::Angle& angle,
                const core::TRCoordinates& position,
                const floordata::ActivationState& activationState,
                bool hasProcessAnimCommandsOverride,
                Characteristics characteristics,
                int16_t darkness);

            virtual ~ItemNode() = default;

            virtual void update() = 0;

            virtual std::shared_ptr<gameplay::Node> getNode() const = 0;

            const core::TRCoordinates& getPosition() const noexcept
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


            int getFloorHeight() const noexcept
            {
                return m_floorHeight;
            }


            void setFloorHeight(const int h) noexcept
            {
                m_floorHeight = h;
            }


            void setCurrentRoom(const loader::Room* newRoom);

            void applyTransform();


            void rotate(const core::Angle dx, const core::Angle dy, const core::Angle dz)
            {
                m_rotation.X += dx;
                m_rotation.Y += dy;
                m_rotation.Z += dz;
            }


            void move(const int dx, const int dy, const int dz)
            {
                m_position.position.X += dx;
                m_position.position.Y += dy;
                m_position.position.Z += dz;
            }


            void moveX(const int d)
            {
                m_position.position.X += d;
            }


            void moveY(const int d)
            {
                m_position.position.Y += d;
            }


            void moveZ(const int d)
            {
                m_position.position.Z += d;
            }


            void setX(const int d)
            {
                m_position.position.X = d;
            }


            void setY(const int d)
            {
                m_position.position.Y = d;
            }


            void setZ(const int d)
            {
                m_position.position.Z = d;
            }


            void move(const glm::vec3& d)
            {
                m_position.position += core::TRCoordinates(d);
            }


            void moveLocal(const int dx, const int dy, const int dz)
            {
                const auto sin = getRotation().Y.sin();
                const auto cos = getRotation().Y.cos();
                m_position.position.X += dz * sin + dx * cos;
                m_position.position.Y += dy;
                m_position.position.Z += dz * cos - dx * sin;
            }


            void setPosition(const core::TRCoordinates& pos)
            {
                m_position.position = pos;
            }


            void setXRotation(const core::Angle x)
            {
                m_rotation.X = x;
            }


            void addXRotation(const core::Angle x)
            {
                m_rotation.X += x;
            }


            void setYRotation(const core::Angle y)
            {
                m_rotation.Y = y;
            }


            void addYRotation(const core::Angle v)
            {
                m_rotation.Y += v;
            }


            void setZRotation(const core::Angle z)
            {
                m_rotation.Z = z;
            }


            void addZRotation(const core::Angle z)
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


            void setFalling(const bool falling) noexcept
            {
                m_falling = falling;
            }


            void setFallSpeed(const int spd)
            {
                m_fallSpeed = spd;
            }


            int getFallSpeed() const noexcept
            {
                return m_fallSpeed;
            }


            void setHorizontalSpeed(const int speed)
            {
                m_horizontalSpeed = speed;
            }


            int getHorizontalSpeed() const
            {
                return m_horizontalSpeed;
            }


            void dampenHorizontalSpeed(const float f)
            {
                m_horizontalSpeed -= m_horizontalSpeed * f;
            }


            virtual void patchFloor(const core::TRCoordinates& /*pos*/, int& /*y*/)
            {
            }


            virtual void patchCeiling(const core::TRCoordinates& /*pos*/, int& /*y*/)
            {
            }


            virtual void onInteract(LaraNode& /*lara*/)
            {
                //BOOST_LOG_TRIVIAL(warning) << "Interaction not implemented: " << m_name;
            }


            void activate();

            void deactivate();

            int getHorizontalSpeed()
            {
                return m_horizontalSpeed;
            }


            int getFallSpeed() noexcept
            {
                return m_fallSpeed;
            }


            virtual bool triggerSwitch(const floordata::ActivationState& arg) = 0;


            std::shared_ptr<audio::SourceHandle> playSoundEffect(int id);


            bool triggerPickUp()
            {
                if (m_triggerState != engine::items::TriggerState::Locked)
                    return false;

                m_triggerState = TriggerState::Activated;
                return true;
            }


            bool triggerKey();


            virtual core::Angle getMovementAngle() const
            {
                return getRotation().Y;
            }


            bool alignTransform(const glm::vec3& trSpeed, const ItemNode& target)
            {
                const auto speed = trSpeed / 16384.0f;
                const auto targetRot = target.getRotation().toMatrix();
                auto targetPos = target.getPosition().toRenderSystem();
                targetPos += glm::vec3(glm::vec4(speed, 0) * targetRot);

                return alignTransformClamped(targetPos, target.getRotation(), 16, 364_au);
            }


            void setRelativeOrientedPosition(const core::TRCoordinates& offset, const ItemNode& target)
            {
                setRotation(target.getRotation());

                const auto r = target.getRotation().toMatrix();
                move(glm::vec3(glm::vec4(offset.toRenderSystem(), 0) * r));
            }


            boost::optional<uint16_t> getCurrentBox() const;

            void updateLighting()
            {
                m_lighting.baseDiff = 0;

                if (m_darkness >= 0)
                {
                    m_lighting.base = (m_darkness - 4096) / 8192.0f;
                    if (m_lighting.base == 0)
                        m_lighting.base = 1;
                    return;
                }

                const auto roomAmbient = 1 - m_position.room->ambientDarkness / 8191.0f;
                BOOST_ASSERT(roomAmbient >= 0 && roomAmbient <= 1);
                m_lighting.base = roomAmbient;

                if (m_position.room->lights.empty())
                {
                    m_lighting.base = 1;
                    m_lighting.baseDiff = 0;
                    return;
                }

                float maxBrightness = 0;
                const auto bboxCtr = m_position.position + getBoundingBox().getCenter();
                for (const auto& light : m_position.room->lights)
                {
                    auto radiusSq = light.radius / 4096.0f;
                    radiusSq *= radiusSq;

                    auto distanceSq = bboxCtr.distanceTo(light.position) / 4096.0f;
                    distanceSq *= distanceSq;

                    const auto lightBrightness = roomAmbient + radiusSq * light.getBrightness() / (radiusSq + distanceSq);
                    if (lightBrightness > maxBrightness)
                    {
                        maxBrightness = lightBrightness;
                        m_lighting.position = light.position.toRenderSystem();
                    }
                }

                m_lighting.base = (roomAmbient + maxBrightness) / 2;
                m_lighting.baseDiff = (maxBrightness - m_lighting.base);

                if (m_lighting.base == 0 && m_lighting.baseDiff == 0)
                    m_lighting.base = 1;
            }


            static const ItemNode* findBaseItemNode(const gameplay::Node& node)
            {
                const ItemNode* item = nullptr;

                auto n = &node;
                while (true)
                {
                    item = dynamic_cast<const ItemNode*>(n);

                    if (item != nullptr || n->getParent().expired())
                        break;

                    n = n->getParent().lock().get();
                }

                return item;
            }


            static void lightBaseBinder(const gameplay::Node& node, gameplay::gl::Program::ActiveUniform& uniform)
            {
                const ItemNode* item = findBaseItemNode(node);

                if (item == nullptr)
                {
                    uniform.set(1.0f);
                    return;
                }

                uniform.set(item->m_lighting.base);
            };


            static void lightBaseDiffBinder(const gameplay::Node& node, gameplay::gl::Program::ActiveUniform& uniform)
            {
                const ItemNode* item = findBaseItemNode(node);

                if (item == nullptr)
                {
                    uniform.set(1.0f);
                    return;
                }

                uniform.set(item->m_lighting.baseDiff);
            };


            static void lightPositionBinder(const gameplay::Node& node, gameplay::gl::Program::ActiveUniform& uniform)
            {
                const ItemNode* item = findBaseItemNode(node);

                if (item == nullptr)
                {
                    static const glm::vec3 invalidPos{ std::numeric_limits<float>::quiet_NaN() };
                    uniform.set(invalidPos);
                    return;
                }

                uniform.set(item->m_lighting.position);
            };

            virtual BoundingBox getBoundingBox() const = 0;

            virtual uint16_t getCurrentState() const = 0;

        protected:
            bool updateActivationTimeout()
            {
                if (!m_activationState.isFullyActivated())
                {
                    return m_activationState.isInverted();
                }

                if (m_activationState.getTimeout() == 0)
                {
                    return !m_activationState.isInverted();
                }

                if (m_activationState.getTimeout() < 0)
                {
                    return m_activationState.isInverted();
                }

                m_activationState.setTimeout(m_activationState.getTimeout() - 1);
                if (m_activationState.getTimeout() <= 0)
                    m_activationState.setTimeout(-1);

                return !m_activationState.isInverted();
            }


            bool alignTransformClamped(const glm::vec3& targetPos, const core::TRRotation& targetRot, const int maxDistance, const core::Angle& maxAngle)
            {
                auto d = targetPos - getPosition().toRenderSystem();
                const auto dist = glm::length(d);
                if (maxDistance < dist)
                {
                    move(static_cast<float>(maxDistance) * glm::normalize(d));
                }
                else
                {
                    setPosition(core::TRCoordinates(targetPos));
                }

                core::TRRotation phi = targetRot - getRotation();
                if (phi.X > maxAngle)
                    addXRotation(maxAngle);
                else if (phi.X < -maxAngle)
                    addXRotation(-maxAngle);
                else
                    addXRotation(phi.X);
                if (phi.Y > maxAngle)
                    addYRotation(maxAngle);
                else if (phi.Y < -maxAngle)
                    addYRotation(-maxAngle);
                else
                    addYRotation(phi.Y);
                if (phi.Z > maxAngle)
                    addZRotation(maxAngle);
                else if (phi.Z < -maxAngle)
                    addZRotation(-maxAngle);
                else
                    addZRotation(phi.Z);

                phi = targetRot - getRotation();
                d = targetPos - getPosition().toRenderSystem();

                return abs(phi.X) < 1_au && abs(phi.Y) < 1_au && abs(phi.Z) < 1_au
                    && abs(d.x) < 1 && abs(d.y) < 1 && abs(d.z) < 1;
            }
        };

        class ModelItemNode : public ItemNode
        {
            std::shared_ptr<SkeletalModelNode> m_skeleton;

        public:
            ModelItemNode(
                const gsl::not_null<level::Level*>& level,
                const std::string& name,
                const gsl::not_null<const loader::Room*>& room,
                const core::Angle& angle,
                const core::TRCoordinates& position,
                const floordata::ActivationState& activationState,
                bool hasProcessAnimCommandsOverride,
                Characteristics characteristics,
                int16_t darkness,
                const loader::SkeletalModelType& animatedModel);


            std::shared_ptr<gameplay::Node> getNode() const override
            {
                return m_skeleton;
            }

            const std::shared_ptr<SkeletalModelNode>& getSkeleton() const
            {
                return m_skeleton;
            }

            bool triggerSwitch(const floordata::ActivationState& arg) override
            {
                if (m_triggerState != engine::items::TriggerState::Activated)
                {
                    return false;
                }

                if (m_skeleton->getCurrentState() != 0 || arg.isLocked())
                {
                    deactivate();
                    m_triggerState = TriggerState::Disabled;
                }
                else
                {
                    m_activationState.setTimeout(arg.getTimeout());
                    m_triggerState = TriggerState::Enabled;
                }

                return true;
            }


            void update() override;

            void applyMovement(bool forLara);

            BoundingBox getBoundingBox() const override;

            uint16_t getCurrentState() const override;
        };

        class SpriteItemNode : public ItemNode
        {
        private:
            std::shared_ptr<gameplay::Node> m_node;

        public:
            SpriteItemNode(
                const gsl::not_null<level::Level*>& level,
                const std::string& name,
                const gsl::not_null<const loader::Room*>& room,
                const core::Angle& angle,
                const core::TRCoordinates& position,
                const floordata::ActivationState& activationState,
                bool hasProcessAnimCommandsOverride,
                Characteristics characteristics,
                int16_t darkness,
                const loader::Sprite& sprite,
                const std::shared_ptr<gameplay::Material>& material,
                const std::vector<std::shared_ptr<gameplay::gl::Texture>>& textures);


            bool triggerSwitch(const floordata::ActivationState&) override
            {
                BOOST_THROW_EXCEPTION(std::runtime_error("triggerSwitch called on sprite"));
            }

            std::shared_ptr<gameplay::Node> getNode() const override
            {
                return m_node;
            }

            void update() override
            {
                // TODO
            }

            BoundingBox getBoundingBox() const override
            {
                BoundingBox bb;
                bb.minX = bb.maxX = getPosition().X;
                bb.minY = bb.maxY = getPosition().Y;
                bb.minZ = bb.maxZ = getPosition().Z;
                return bb;
            }

            uint16_t getCurrentState() const override
            {
                // TODO
                return 0;
            }
        };
    }
}
