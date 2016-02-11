#include "entity.h"

#include "character_controller.h"
#include "ragdoll.h"
#include "room.h"

#include <boost/log/trivial.hpp>

namespace world
{
void Entity::enable()
{
    if(!m_enabled)
    {
        m_skeleton.enableCollision();
        m_enabled = m_active = m_visible = true;
    }
}

void Entity::disable()
{
    if(m_enabled)
    {
        m_skeleton.disableCollision();
        m_active = m_enabled = m_visible = false;
    }
}

/**
 * It is from bullet_character_controller
 */
bool Entity::getPenetrationFixVector(btPairCachingGhostObject& ghost, btManifoldArray& manifoldArray, irr::core::vector3df& correction) const
{
    // Here we must refresh the overlapping paircache as the penetrating movement itself or the
    // previous recovery iteration might have used setWorldTransform and pushed us into an object
    // that is not in the previous cache contents from the last timestep, as will happen if we
    // are pushed into a new AABB overlap. Unhandled this means the next convex sweep gets stuck.
    //
    // Do this by calling the broadphase's setAabb with the moved AABB, this will update the broadphase
    // paircache and the ghostobject's internal paircache at the same time.    /BW

    btBroadphasePairArray &pairArray = ghost.getOverlappingPairCache()->getOverlappingPairArray();

    btVector3 aabb_min, aabb_max;
    ghost.getCollisionShape()->getAabb(ghost.getWorldTransform(), aabb_min, aabb_max);
    getWorld()->m_engine->m_bullet.dynamicsWorld->getBroadphase()->setAabb(ghost.getBroadphaseHandle(), aabb_min, aabb_max, getWorld()->m_engine->m_bullet.dynamicsWorld->getDispatcher());
    getWorld()->m_engine->m_bullet.dynamicsWorld->getDispatcher()->dispatchAllCollisionPairs(ghost.getOverlappingPairCache(), getWorld()->m_engine->m_bullet.dynamicsWorld->getDispatchInfo(), getWorld()->m_engine->m_bullet.dynamicsWorld->getDispatcher());

    correction = { 0,0,0 };
    int num_pairs = ghost.getOverlappingPairCache()->getNumOverlappingPairs();
    bool ret = false;
    for(int i = 0; i < num_pairs; i++)
    {
        manifoldArray.clear();
        // do not use commented code: it prevents to collision skips.
        //btBroadphasePair &pair = pairArray[i];
        //btBroadphasePair* collisionPair = bt_engine_dynamicsWorld->getPairCache()->findPair(pair.m_pProxy0,pair.m_pProxy1);
        btBroadphasePair *collisionPair = &pairArray[i];

        if(!collisionPair)
        {
            continue;
        }

        if(collisionPair->m_algorithm)
        {
            collisionPair->m_algorithm->getAllContactManifolds(manifoldArray);
        }

        int manifolds_size = manifoldArray.size();
        for(int j = 0; j < manifolds_size; j++)
        {
            btPersistentManifold* manifold = manifoldArray[j];
            Object* obj0 = static_cast<Object*>(manifold->getBody0()->getUserPointer());
            Object* obj1 = static_cast<Object*>(manifold->getBody1()->getUserPointer());
            if(obj0->getCollisionType() == world::CollisionType::Ghost || obj1->getCollisionType() == world::CollisionType::Ghost)
            {
                continue;
            }
            for(int k = 0; k < manifold->getNumContacts(); k++)
            {
                const btManifoldPoint&pt = manifold->getContactPoint(k);
                auto dist = pt.getDistance();

                if(dist < 0.0)
                {
                    correction -= util::convert(pt.m_normalWorldOnB) * dist;
                    ret = true;
                }
            }
        }
    }

    return ret;
}

void Entity::ghostUpdate()
{
    if(!m_skeleton.hasGhosts())
        return;

    if(m_typeFlags & ENTITY_TYPE_DYNAMIC)
    {
        for(const animation::Bone& bone : m_skeleton.getBones())
        {
            auto tr = m_transform * bone.globalTransform;
            bone.ghostObject->getWorldTransform().setFromOpenGLMatrix(tr.pointer());
            auto pos = bone.mesh->m_center;
            tr.transformVect(pos);
            bone.ghostObject->getWorldTransform().setOrigin(util::convert(pos));
        }
    }
    else
    {
        for(const animation::Bone& bone : m_skeleton.getBones())
        {
            auto tr = bone.bt_body->getWorldTransform();
            tr.setOrigin(tr * util::convert(bone.mesh->m_center));
            bone.ghostObject->getWorldTransform() = tr;
        }
    }
}

///@TODO: make experiment with convexSweepTest with spheres: no more iterative cycles;
int Entity::getPenetrationFixVector(irr::core::vector3df& reaction, bool hasMove)
{
    reaction = { 0,0,0 };
    if(!m_skeleton.hasGhosts() || m_skeleton.getModel()->m_noFixAll)
        return 0;

    auto orig_pos = m_transform[3];
    int ret = 0;
    for(size_t boneIndex = 0; boneIndex < m_skeleton.getModel()->getCollisionMapSize(); boneIndex++)
    {
        const animation::Bone& bone = m_skeleton.getBones()[m_skeleton.getModel()->getCollisionMap(boneIndex)];

        if(bone.body_part & m_skeleton.getModel()->m_noFixBodyParts)
        {
            continue;
        }

        // antitunneling condition for main body parts, needs only in move case: ((move != NULL) && (btag->body_part & (BODY_PART_BODY_LOW | BODY_PART_BODY_UPPER)))
        irr::core::vector3df from;
        if(!bone.parent || (hasMove && (bone.body_part & (BODY_PART_BODY_LOW | BODY_PART_BODY_UPPER))))
        {
            BOOST_ASSERT(bone.ghostObject);
            from = util::convert(bone.ghostObject->getWorldTransform().getOrigin());
            from += irr::core::vector3df(m_transform[3] - orig_pos);
        }
        else
        {
            auto parent_from = bone.parent->globalTransform * bone.parent->mesh->m_center;
            from = irr::core::vector3df(m_transform * parent_from);
        }

        auto tr = m_transform * bone.globalTransform;
        auto to = tr * bone.mesh->m_center;
        auto curr = from;
        auto move = to - from;
        auto move_len = move.getLength();
        if(boneIndex == 0 && move_len > 1024.0)                                 ///@FIXME: magick const 1024.0!
        {
            break;
        }
        int iter = static_cast<int>(4.0 * move_len / bone.mesh->m_radius + 1);     ///@FIXME (not a critical): magick const 4.0!
        move /= static_cast<irr::f32>(iter);

        for(int j = 0; j <= iter; j++)
        {
            tr.setTranslation(curr);
            bone.ghostObject->getWorldTransform().setFromOpenGLMatrix(tr.pointer());
            irr::core::vector3df tmp;
            if(getPenetrationFixVector(*bone.ghostObject, m_skeleton.manifoldArray(), tmp))
            {
                m_transform.setTranslation(m_transform.getTranslation() + tmp);
                curr += tmp;
                from += tmp;
                ret++;
            }
            curr += move;
        }
    }
    reaction = irr::core::vector3df(m_transform[3] - orig_pos);
    m_transform[3] = orig_pos;

    return ret;
}

void Entity::fixPenetrations(const irr::core::vector3df* move)
{
    if(!m_skeleton.hasGhosts())
        return;

    if(m_typeFlags & ENTITY_TYPE_DYNAMIC)
    {
        return;
    }

    if(m_skeleton.getModel()->m_noFixAll)
    {
        ghostUpdate();
        return;
    }

    irr::core::vector3df reaction;
    getPenetrationFixVector(reaction, move != nullptr);
    m_transform.setTranslation( m_transform.getTranslation() + reaction);

    ghostUpdate();
}

void Entity::transferToRoom(Room* room)
{
    if(!getRoom() || getRoom()->overlaps(room))
        return;

    getRoom()->removeEntity(this);

    if(room)
        room->addEntity(this);
}

std::shared_ptr<engine::BtEngineClosestConvexResultCallback> Entity::callbackForCamera() const
{
    auto cb = std::make_shared<engine::BtEngineClosestConvexResultCallback>(this);
    cb->m_collisionFilterMask = btBroadphaseProxy::StaticFilter | btBroadphaseProxy::KinematicFilter;
    return cb;
}

void Entity::checkCollisionCallbacks()
{
    if(!m_skeleton.hasGhosts())
        return;

    uint32_t curr_flag;
    m_skeleton.updateCurrentCollisions(*this, m_transform);
    while(btCollisionObject* cobj = m_skeleton.getRemoveCollisionBodyParts(0xFFFFFFFF, curr_flag))
    {
        // do callbacks here:
        Object* object = static_cast<Object*>(cobj->getUserPointer());

        if(Entity* activator = dynamic_cast<Entity*>(object))
        {
            if(activator->m_callbackFlags & ENTITY_CALLBACK_COLLISION)
            {
                // Activator and entity IDs are swapped in case of collision callback.
                getWorld()->m_engine->m_scriptEngine.execEntity(ENTITY_CALLBACK_COLLISION, activator->getId(), getId());
                //ConsoleInfo::instance().printf("char_body_flag = 0x%X, collider_type = %d", curr_flag, type);
            }
        }
        else if((m_callbackFlags & ENTITY_CALLBACK_ROOMCOLLISION) && dynamic_cast<Room*>(object))
        {
            getWorld()->m_engine->m_scriptEngine.execEntity(ENTITY_CALLBACK_ROOMCOLLISION, getId(), object->getId());
        }
    }
}

bool Entity::wasCollisionBodyParts(uint32_t parts_flags) const
{
    if(!m_skeleton.hasGhosts())
        return false;

    for(const animation::Bone& bone : m_skeleton.getBones())
    {
        if((bone.body_part & parts_flags) && !bone.last_collisions.empty())
        {
            return true;
        }
    }

    return false;
}

void Entity::updateRoomPos()
{
    irr::core::vector3df pos = getRoomPos();
    auto new_room = getWorld()->Room_FindPosCogerrence(pos, getRoom());
    if(!new_room)
    {
        m_currentSector = nullptr;
        return;
    }

    const RoomSector* new_sector = new_room->getSectorXYZ(pos);
    if(new_room != new_sector->owner_room)
    {
        new_room = new_sector->owner_room;
    }

    transferToRoom(new_room);

    setRoom(new_room);
    m_lastSector = m_currentSector;

    if(m_currentSector != new_sector)
    {
        m_triggerLayout &= static_cast<uint8_t>(~ENTITY_TLAYOUT_SSTATUS); // Reset sector status.
        m_currentSector = new_sector;
    }
}

void Entity::updateRigidBody(bool force)
{
    if(m_typeFlags & ENTITY_TYPE_DYNAMIC)
    {
        m_skeleton.getBones()[0].bt_body->getWorldTransform().getOpenGLMatrix(m_transform.pointer());
        updateRoomPos();
        m_skeleton.updateTransform(m_transform);
        updateGhostRigidBody();
        m_skeleton.updateBoundingBox();
    }
    else
    {
        if(m_skeleton.getModel() == nullptr || (!force && m_skeleton.getModel()->isStaticAnimation()))
        {
            return;
        }

        updateRoomPos();
        if(getCollisionType() != world::CollisionType::Static)
        {
            m_skeleton.updateRigidBody(m_transform);
        }
    }
    rebuildBoundingBox();
}

void Entity::updateTransform()
{
    m_angles.X = util::wrapAngle(m_angles.X);
    m_angles.Y = util::wrapAngle(m_angles.Y);
    m_angles.Z = util::wrapAngle(m_angles.Z);

    m_transform.setRotationDegrees({m_angles.Z, m_angles.Y, m_angles.X});
}

void Entity::updateCurrentSpeed(bool zeroVz)
{
    irr::f32 t = m_currentSpeed * animation::AnimationFrameRate;
    irr::f32 vz = zeroVz ? 0.0f : m_speed.Z;

    if(m_moveDir == MoveDirection::Forward)
    {
        m_speed = irr::core::vector3df(m_transform[1]) * t;
    }
    else if(m_moveDir == MoveDirection::Backward)
    {
        m_speed = irr::core::vector3df(m_transform[1]) * -t;
    }
    else if(m_moveDir == MoveDirection::Left)
    {
        m_speed = irr::core::vector3df(m_transform[0]) * -t;
    }
    else if(m_moveDir == MoveDirection::Right)
    {
        m_speed = irr::core::vector3df(m_transform[0]) * t;
    }
    else
    {
        m_speed = { 0,0,0 };
    }

    m_speed.Z = vz;
}

void Entity::addOverrideAnim(const std::shared_ptr<animation::SkeletalModel>& model)
{
    if(!model || model->getBoneCount() != m_skeleton.getBoneCount())
        return;

    m_skeleton.setModel(model);
}

irr::f32 Entity::findDistance(const Entity& other)
{
    return m_transform.getTranslation().getDistanceFrom(other.m_transform.getTranslation());
}

/**
 * Callback to handle anim commands
 * @param command
 */
void Entity::doAnimCommand(const animation::AnimCommand& command)
{
    switch(command.cmdId)
    {
        case animation::AnimCommandOpcode::SetPosition:    // (tr_x, tr_y, tr_z)
        {
            // x=x, y=z, z=-y
            const irr::f32 x = irr::f32(command.param[0]);
            const irr::f32 y = irr::f32(command.param[2]);
            const irr::f32 z = -irr::f32(command.param[1]);
            irr::core::vector3df ofs(x, y, z);
            m_transform.setTranslation(m_transform.getTranslation() + m_transform * ofs);
        }
        break;

        case animation::AnimCommandOpcode::SetVelocity:    // (float vertical, float horizontal)
        {
            irr::f32 vert;
            const irr::f32 horiz = irr::f32(command.param[1]);
            if(btFuzzyZero(m_vspeed_override))
            {
                vert = -irr::f32(command.param[0]);
            }
            else
            {
                vert = m_vspeed_override;
                m_vspeed_override = 0.0f;
            }
            jump(vert, horiz);
        }
        break;

        case animation::AnimCommandOpcode::EmptyHands:
            // This command is used only for Lara.
            // Reset interaction-blocking
            break;

        case animation::AnimCommandOpcode::Kill:
            // This command is usually used only for non-Lara items,
            // although there seem to be Lara-anims with this cmd id (tr4 anim 415, shotgun overlay)
            // TODO: for switches, this command indicates the trigger-frame
            if(!isPlayer())
            {
                kill();
            }
            break;

        case animation::AnimCommandOpcode::PlaySound:      // (sndParam)
        {
            audio::SoundId soundId = command.param[0] & 0x3FFF;

            // Quick workaround for TR3 quicksand.
            if(getSubstanceState() == Substance::QuicksandConsumed || getSubstanceState() == Substance::QuicksandShallow)
            {
                soundId = audio::SoundWadeShallow;
            }

            if(command.param[0] & animation::TR_ANIMCOMMAND_CONDITION_WATER)
            {
                if(getSubstanceState() == Substance::WaterShallow)
                    getWorld()->m_engine->m_audioEngine.send(soundId, audio::EmitterType::Entity, getId());
            }
            else if(command.param[0] & animation::TR_ANIMCOMMAND_CONDITION_LAND)
            {
                if(getSubstanceState() != Substance::WaterShallow)
                    getWorld()->m_engine->m_audioEngine.send(soundId, audio::EmitterType::Entity, getId());
            }
            else
            {
                getWorld()->m_engine->m_audioEngine.send(soundId, audio::EmitterType::Entity, getId());
            }
        }
        break;

        case animation::AnimCommandOpcode::PlayEffect:     // (flipeffectParam)
        {
            const uint16_t effect_id = command.param[0] & 0x3FFF;
            if(effect_id == 0)  // rollflip
            {
                // FIXME: wrapping angles are bad for quat lerp:
                m_angles.X += 180.0;

                if(m_moveType == MoveType::Underwater)
                {
                    m_angles.Y = -m_angles.Y;                         // for underwater case
                }
                if(m_moveDir == MoveDirection::Backward)
                {
                    m_moveDir = MoveDirection::Forward;
                }
                else if(m_moveDir == MoveDirection::Forward)
                {
                    m_moveDir = MoveDirection::Backward;
                }
                updateTransform();
            }
            else
            {
                getWorld()->m_engine->m_scriptEngine.execEffect(effect_id, getId());
            }
        }
        break;

        default:
            // Unknown command
            break;
    }
}

void Entity::processSector()
{
    if(!m_currentSector)
        return;

    // Calculate both above and below sectors for further usage.
    // Sector below is generally needed for getting proper trigger index,
    // as many triggers tend to be called from the lowest room in a row
    // (e.g. first trapdoor in The Great Wall, etc.)
    // Sector above primarily needed for paranoid cases of monkeyswing.

    BOOST_ASSERT(m_currentSector != nullptr);
    const RoomSector* lowest_sector = m_currentSector->getLowestSector();
    BOOST_ASSERT(lowest_sector != nullptr);

    processSectorImpl();

    // If entity either marked as trigger activator (Lara) or heavytrigger activator (other entities),
    // we try to execute a trigger for this sector.

    if((m_typeFlags & ENTITY_TYPE_TRIGGER_ACTIVATOR) || (m_typeFlags & ENTITY_TYPE_HEAVYTRIGGER_ACTIVATOR))
    {
        // Look up trigger function table and run trigger if it exists.
        try
        {
            if(getWorld()->m_engine->m_scriptEngine["tlist_RunTrigger"].is<lua::Callable>())
                getWorld()->m_engine->m_scriptEngine["tlist_RunTrigger"].call(int(lowest_sector->trig_index), m_skeleton.getModel()->getId() == 0 ? TR_ACTIVATORTYPE_LARA : TR_ACTIVATORTYPE_MISC, getId());
        }
        catch(lua::RuntimeError& error)
        {
            BOOST_LOG_TRIVIAL(error) << error.what();
        }
    }
}

void Entity::setAnimation(animation::AnimationId animation, int frame)
{
    m_skeleton.setAnimation(animation, frame);

    m_skeleton.getModel()->m_noFixAll = false;

    // some items (jeep) need this here...
    m_skeleton.updatePose();
    //    updateRigidBody(false);
}

boost::optional<size_t> Entity::findTransitionCase(LaraState id) const
{
    const animation::Transition* transition = m_skeleton.getCurrentAnimation().findTransitionForState(id);
    if(!transition)
        return boost::none;

    for(size_t j = 0; j < transition->cases.size(); j++)
    {
        const animation::TransitionCase& transitionCase = transition->cases[j];

        if(transitionCase.lastFrame >= transitionCase.firstFrame
           && m_skeleton.getCurrentFrame() >= transitionCase.firstFrame
           && m_skeleton.getCurrentFrame() <= transitionCase.lastFrame)
        {
            return j;
        }
    }

    return boost::none;
}

void Entity::updateInterpolation()
{
    if((m_typeFlags & ENTITY_TYPE_DYNAMIC) != 0)
        return;

    // Bone animation interp:
    m_skeleton.updatePose();
}

animation::AnimUpdate Entity::advanceTime(util::Duration time)
{
    if((m_typeFlags & ENTITY_TYPE_DYNAMIC)
       || !m_active
       || !m_enabled
       || m_skeleton.getModel() == nullptr
       || m_skeleton.getModel()->isStaticAnimation())
    {
        return animation::AnimUpdate::None;
    }
    if(m_skeleton.getAnimationMode() == animation::AnimationMode::Locked)
        return animation::AnimUpdate::NewFrame;  // penetration fix will be applyed in Character_Move... functions

    animation::AnimUpdate stepResult = m_skeleton.advanceTime(time, this);

    //    setAnimation(m_bf.animations.current_animation, m_bf.animations.current_frame);

    m_skeleton.updatePose();
    fixPenetrations(nullptr);

    return stepResult;
}

/**
 * Entity framestep actions
 * @param time      frame time
 */
void Entity::frame(util::Duration time)
{
    if(!m_enabled)
    {
        return;
    }
    if(m_typeFlags & ENTITY_TYPE_DYNAMIC)
    {
        // Ragdoll
        updateRigidBody(false); // bbox update, room update, m_transform from btBody...
        return;
    }

    fixPenetrations(nullptr);
    processSector();    // triggers
    getWorld()->m_engine->m_scriptEngine.loopEntity(getId());

    if(m_typeFlags & ENTITY_TYPE_COLLCHECK)
        checkCollisionCallbacks();

    advanceTime(time);

    // TODO: check rigidbody update requirements.
    //       If m_transform changes, rigid body must be updated regardless of anim frame change...
    //if(animStepResult != ENTITY_ANIM_NONE)
    //{ }
    m_skeleton.updatePose();
    updateRigidBody(false);
}

/**
 * The function rebuild / renew entity's BV
 */
void Entity::rebuildBoundingBox()
{
    if(!m_skeleton.getModel())
        return;

    m_obb.rebuild(m_skeleton.getBoundingBox());
    m_obb.doTransform();
}

void Entity::checkActivators()
{
    if(getRoom() == nullptr)
        return;

    irr::core::vector3df ppos = m_transform.getTranslation() + m_transform[1] * m_skeleton.getBoundingBox().max.Y;
    auto containers = getRoom()->getObjects();
    for(Object* object : containers)
    {
        Entity* e = dynamic_cast<Entity*>(object);
        if(!e)
            continue;

        if(!e->m_enabled)
            continue;

        if(e->m_typeFlags & ENTITY_TYPE_INTERACTIVE)
        {
            //Mat4_vec3_mul_macro(pos, e->transform, e->activation_offset);
            if(e != this && core::testOverlap(*e, *this)) //(vec3_dist_sq(transform+12, pos) < r))
            {
                getWorld()->m_engine->m_scriptEngine.execEntity(ENTITY_CALLBACK_ACTIVATE, e->getId(), getId());
            }
        }
        else if(e->m_typeFlags & ENTITY_TYPE_PICKABLE)
        {
            irr::f32 r = e->m_activationRadius;
            r *= r;
            const auto v = e->m_transform.getTranslation();
            if(e != this
               && (v.X - ppos.X) * (v.X - ppos.X) + (v.Y - ppos.Y) * (v.Y - ppos.Y) < r
               && v.Y + 32.0 > m_transform.getTranslation().Z + m_skeleton.getBoundingBox().min.Z
               && v.Z - 32.0 < m_transform.getTranslation().Z + m_skeleton.getBoundingBox().max.Z)
            {
                getWorld()->m_engine->m_scriptEngine.execEntity(ENTITY_CALLBACK_ACTIVATE, e->getId(), getId());
            }
        }
    }
}

void Entity::moveForward(irr::f32 dist)
{
    m_transform[3] += m_transform[1] * dist;
}

void Entity::moveStrafe(irr::f32 dist)
{
    m_transform[3] += m_transform[0] * dist;
}

void Entity::moveVertical(irr::f32 dist)
{
    m_transform[3] += m_transform[2] * dist;
}

Entity::Entity(ObjectId id, World* world)
    : Object(id, world)
    , m_skeleton(this)
{
    m_obb.transform = &m_transform;
}

Entity::~Entity()
{
    if(!m_skeleton.getModel()->m_btJoints.empty())
    {
        deleteRagdoll();
    }
}

bool Entity::createRagdoll(RagdollSetup* setup)
{
    // No entity, setup or body count overflow - bypass function.

    if(!setup || setup->body_setup.size() > m_skeleton.getBoneCount())
    {
        return false;
    }

    bool result = true;

    // If ragdoll already exists, overwrite it with new one.

    if(!m_skeleton.getModel()->m_btJoints.empty())
    {
        result = deleteRagdoll();
    }

    // Setup bodies.
    m_skeleton.getModel()->m_btJoints.clear();
    // update current character animation and full fix body to avoid starting ragdoll partially inside the wall or floor...
    m_skeleton.updatePose();
    m_skeleton.getModel()->m_noFixAll = false;
    m_skeleton.getModel()->m_noFixBodyParts = 0x00000000;
    fixPenetrations(nullptr);

    result &= m_skeleton.createRagdoll(*setup);

    updateRigidBody(true);

    m_skeleton.initCollisions(m_speed);

    // Setup constraints.
    m_skeleton.getModel()->m_btJoints.resize(setup->joint_setup.size());

    for(size_t i = 0; i < setup->joint_setup.size(); i++)
    {
        if(setup->joint_setup[i].body_index >= m_skeleton.getBoneCount() || !m_skeleton.getBones()[setup->joint_setup[i].body_index].bt_body)
        {
            result = false;
            break;       // If body 1 or body 2 are absent, return false and bypass this joint.
        }

        const animation::Bone* btB = &m_skeleton.getBones()[setup->joint_setup[i].body_index];
        const animation::Bone* btA = btB->parent;
        if(!btA)
        {
            result = false;
            break;
        }
        btTransform localA;
        localA.getBasis().setEulerZYX(setup->joint_setup[i].body1_angle[0], setup->joint_setup[i].body1_angle[1], setup->joint_setup[i].body1_angle[2]);
        localA.setOrigin(util::convert(irr::core::vector3df(btB->localTransform[3])));

        btTransform localB;
        localB.getBasis().setEulerZYX(setup->joint_setup[i].body2_angle[0], setup->joint_setup[i].body2_angle[1], setup->joint_setup[i].body2_angle[2]);
        localB.setOrigin({ 0, 0, 0 });

        switch(setup->joint_setup[i].joint_type)
        {
            case RagdollJointSetup::Point:
            {
                m_skeleton.getModel()->m_btJoints[i] = std::make_shared<btPoint2PointConstraint>(*m_skeleton.getBones()[btA->index].bt_body, *m_skeleton.getBones()[btB->index].bt_body, localA.getOrigin(), localB.getOrigin());
            }
            break;

            case RagdollJointSetup::Hinge:
            {
                std::shared_ptr<btHingeConstraint> hingeC = std::make_shared<btHingeConstraint>(*m_skeleton.getBones()[btA->index].bt_body, *m_skeleton.getBones()[btB->index].bt_body, localA, localB);
                hingeC->setLimit(setup->joint_setup[i].joint_limit[0], setup->joint_setup[i].joint_limit[1], 0.9f, 0.3f, 0.3f);
                m_skeleton.getModel()->m_btJoints[i] = hingeC;
            }
            break;

            case RagdollJointSetup::Cone:
            {
                std::shared_ptr<btConeTwistConstraint> coneC = std::make_shared<btConeTwistConstraint>(*m_skeleton.getBones()[btA->index].bt_body, *m_skeleton.getBones()[btB->index].bt_body, localA, localB);
                coneC->setLimit(setup->joint_setup[i].joint_limit[0], setup->joint_setup[i].joint_limit[1], setup->joint_setup[i].joint_limit[2], 0.9f, 0.3f, 0.7f);
                m_skeleton.getModel()->m_btJoints[i] = coneC;
            }
            break;
        }

        m_skeleton.getModel()->m_btJoints[i]->setParam(BT_CONSTRAINT_STOP_CFM, setup->joint_cfm, -1);
        m_skeleton.getModel()->m_btJoints[i]->setParam(BT_CONSTRAINT_STOP_ERP, setup->joint_erp, -1);

        m_skeleton.getModel()->m_btJoints[i]->setDbgDrawSize(64.0);
        getWorld()->m_engine->m_bullet.dynamicsWorld->addConstraint(m_skeleton.getModel()->m_btJoints[i].get(), true);
    }

    if(!result)
    {
        deleteRagdoll();  // PARANOID: Clean up the mess, if something went wrong.
    }
    else
    {
        m_typeFlags |= ENTITY_TYPE_DYNAMIC;
    }
    return result;
}

bool Entity::deleteRagdoll()
{
    if(m_skeleton.getModel()->m_btJoints.empty())
        return false;

    for(std::shared_ptr<btTypedConstraint> joint : m_skeleton.getModel()->m_btJoints)
    {
        if(joint)
        {
            getWorld()->m_engine->m_bullet.dynamicsWorld->removeConstraint(joint.get());
            joint.reset();
        }
    }

    for(const animation::Bone& bone : m_skeleton.getBones())
    {
        getWorld()->m_engine->m_bullet.dynamicsWorld->removeRigidBody(bone.bt_body.get());
        bone.bt_body->setMassProps(0, btVector3(0.0, 0.0, 0.0));
        getWorld()->m_engine->m_bullet.dynamicsWorld->addRigidBody(bone.bt_body.get(), COLLISION_GROUP_KINEMATIC, COLLISION_MASK_ALL);
        if(bone.ghostObject)
        {
            getWorld()->m_engine->m_bullet.dynamicsWorld->removeCollisionObject(bone.ghostObject.get());
            getWorld()->m_engine->m_bullet.dynamicsWorld->addCollisionObject(bone.ghostObject.get(), COLLISION_GROUP_CHARACTERS, COLLISION_GROUP_ALL);
        }
    }

    m_skeleton.getModel()->m_btJoints.clear();

    m_typeFlags &= ~ENTITY_TYPE_DYNAMIC;

    return true;

    // NB! Bodies remain in the same state!
    // To make them static again, additionally call setEntityBodyMass script function.
}

irr::core::vector3df Entity::applyGravity(util::Duration time)
{
    const auto gravityAccelleration = util::convert(getWorld()->m_engine->m_bullet.dynamicsWorld->getGravity());
    const auto gravitySpeed = gravityAccelleration * util::toSeconds(time);
    auto move = (m_speed + gravitySpeed*0.5f) * util::toSeconds(time);
    m_speed += gravitySpeed;
    return move;
}
} // namespace world
