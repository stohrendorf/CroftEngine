#pragma once

#include <cstdint>

#include <BulletDynamics/Dynamics/btRigidBody.h>

#include "engine/engine.h"
#include "world/entity.h"
#include "world/world.h"

namespace world
{
#define HAIR_TR1       0
#define HAIR_TR2       1
#define HAIR_TR3       2
#define HAIR_TR4_KID_1 3
#define HAIR_TR4_KID_2 4
#define HAIR_TR4_OLD   5
#define HAIR_TR5_KID_1 6
#define HAIR_TR5_KID_2 7
#define HAIR_TR5_OLD   8

using HairElementId = uint32_t;

struct HairElement
{
    std::shared_ptr<core::BaseMesh> mesh;           // Pointer to rendered mesh.
    std::unique_ptr<btCollisionShape> shape;          // Pointer to collision shape.
    std::shared_ptr<btRigidBody> body;           // Pointer to dynamic body.
    irr::core::vector3df position;     // Position of this hair element
    // relative to the model (NOT the parent!). Should be a matrix in theory,
    // but since this never has a rotation part, we can save a few bytes here.
};

struct HairSetup;

struct Hair : public Object
{
    const Entity* m_ownerChar = nullptr;         // Entity who owns this hair.
    animation::BoneId m_ownerBody = 0;         // Owner entity's body ID.
    irr::core::matrix4 m_ownerBodyHairRoot; // transform from owner body to root of hair start

    HairElementId m_rootIndex = 0;         // Index of "root" element.
    HairElementId m_tailIndex = 0;         // Index of "tail" element.

    std::vector<HairElement> m_elements;           // Array of elements.

    std::vector<std::unique_ptr<btGeneric6DofConstraint>> m_joints;             // Array of joints.

    std::shared_ptr<core::BaseMesh> m_mesh = nullptr;               // Mesh containing all vertices of all parts of this hair object.

    explicit Hair(World* world, Room* room = nullptr)
        : Object(0, world, room)
    {
    }

    ~Hair();

    // Creates hair into allocated hair structure, using previously defined setup and
    // entity index.
    bool create(HairSetup* setup, const Entity& parent_entity);

private:
    void createHairMesh(const animation::SkeletalModel& model);
};

struct HairSetup
{
    animation::ModelId m_model;              // Hair model ID
    animation::BoneId m_linkBody;          // Lara's head mesh index

    irr::f32     m_rootWeight;        // Root and tail hair body weight. Intermediate body
    irr::f32     m_tailWeight;        // weights are calculated from these two parameters

    irr::f32     m_hairDamping[2];    // Damping affects hair "plasticity"
    irr::f32     m_hairInertia;       // Inertia affects hair "responsiveness"
    irr::f32     m_hairRestitution;   // "Bounciness" of the hair
    irr::f32     m_hairFriction;      // How much other bodies will affect hair trajectory

    irr::f32     m_jointOverlap;      // How much two hair bodies overlap each other

    irr::f32     m_jointCfm;          // Constraint force mixing (joint softness)
    irr::f32     m_jointErp;          // Error reduction parameter (joint "inertia")

    irr::core::vector3df m_headOffset;        // Linear offset to place hair to
    irr::core::vector3df m_rootAngle;      // First constraint set angle (to align hair angle)

    // Gets scripted hair set-up to specified hair set-up structure.
    void getSetup(world::World& world, int hair_entry_index);
};
} // namespace world
