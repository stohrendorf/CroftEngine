#include "objectfactory.h"

#include "animating.h"
#include "atlanteanlava.h"
#include "barricade.h"
#include "bat.h"
#include "bear.h"
#include "block.h"
#include "boulder.h"
#include "bridgeflat.h"
#include "collapsiblefloor.h"
#include "crocodile.h"
#include "cutsceneactors.h"
#include "dart.h"
#include "dartgun.h"
#include "door.h"
#include "doppelganger.h"
#include "earthquake.h"
#include "engine/items_tr1.h"
#include "engine/presenter.h"
#include "engine/world.h"
#include "flameemitter.h"
#include "gorilla.h"
#include "keyhole.h"
#include "laraobject.h"
#include "larson.h"
#include "lavaparticleemitter.h"
#include "lightningball.h"
#include "lion.h"
#include "mummy.h"
#include "mutant.h"
#include "mutantegg.h"
#include "pickupobject.h"
#include "pierre.h"
#include "puzzlehole.h"
#include "raptor.h"
#include "rat.h"
#include "render/scene/materialmanager.h"
#include "scionpiece.h"
#include "slammingdoors.h"
#include "slopedbridge.h"
#include "stubobject.h"
#include "swingingblade.h"
#include "switch.h"
#include "swordofdamocles.h"
#include "tallblock.h"
#include "teethspikes.h"
#include "thorhammer.h"
#include "trapdoordown.h"
#include "trapdoorup.h"
#include "trex.h"
#include "underwaterswitch.h"
#include "waterfallmist.h"
#include "wolf.h"

#include <boost/log/trivial.hpp>

namespace engine::objects
{
std::shared_ptr<Object> createObject(World& world, loader::file::Item& item)
{
  const auto* room = &world.getRooms().at(item.room.get());

  if(const auto& model = world.findAnimatedModelForType(item.type))
  {
    std::shared_ptr<Object> object;

    if(item.type == TR1ItemId::Lara)
    {
      object = std::make_shared<LaraObject>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::Wolf)
    {
      object = std::make_shared<Wolf>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::Bear)
    {
      object = std::make_shared<Bear>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::Bat)
    {
      object = std::make_shared<Bat>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::FallingBlock)
    {
      object = std::make_shared<CollapsibleFloor>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::SwingingBlade)
    {
      object = std::make_shared<SwingingBlade>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::RollingBall)
    {
      object = std::make_shared<RollingBall>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::Dart)
    {
      object = std::make_shared<Dart>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::DartEmitter)
    {
      object = std::make_shared<DartGun>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::LiftingDoor)
    {
      object = std::make_shared<TrapDoorUp>(&world, room, item, model.get());
    }
    else if(item.type >= TR1ItemId::PushableBlock1 && item.type <= TR1ItemId::PushableBlock4)
    {
      object = std::make_shared<Block>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::MovingBlock)
    {
      object = std::make_shared<TallBlock>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::WallSwitch)
    {
      object = std::make_shared<Switch>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::UnderwaterSwitch)
    {
      object = std::make_shared<UnderwaterSwitch>(&world, room, item, model.get());
    }
    else if(item.type >= TR1ItemId::Door1 && item.type <= TR1ItemId::Door8)
    {
      object = std::make_shared<Door>(&world, room, item, model.get());
    }
    else if(item.type >= TR1ItemId::Trapdoor1 && item.type <= TR1ItemId::Trapdoor2)
    {
      object = std::make_shared<TrapDoorDown>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::BridgeFlat)
    {
      object = std::make_shared<BridgeFlat>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::BridgeTilt1)
    {
      object = std::make_shared<BridgeSlope1>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::BridgeTilt2)
    {
      object = std::make_shared<BridgeSlope2>(&world, room, item, model.get());
    }
    else if(item.type >= TR1ItemId::Keyhole1 && item.type <= TR1ItemId::Keyhole4)
    {
      object = std::make_shared<KeyHole>(&world, room, item, model.get());
    }
    else if(item.type >= TR1ItemId::PuzzleHole1 && item.type <= TR1ItemId::PuzzleHole4)
    {
      object = std::make_shared<PuzzleHole>(&world, room, item, model.get());
    }
    else if(item.type >= TR1ItemId::Animating1 && item.type <= TR1ItemId::Animating3)
    {
      object = std::make_shared<Animating>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::TeethSpikes)
    {
      object = std::make_shared<TeethSpikes>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::Raptor)
    {
      object = std::make_shared<Raptor>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::SwordOfDamocles || item.type == TR1ItemId::FallingCeiling)
    {
      object = std::make_shared<SwordOfDamocles>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::CutsceneActor1)
    {
      object = std::make_shared<CutsceneActor1>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::CutsceneActor2)
    {
      object = std::make_shared<CutsceneActor2>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::CutsceneActor3)
    {
      object = std::make_shared<CutsceneActor3>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::CutsceneActor4)
    {
      object = std::make_shared<CutsceneActor4>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::WaterfallMist)
    {
      object = std::make_shared<WaterfallMist>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::TRex)
    {
      object = std::make_shared<TRex>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::Mummy)
    {
      object = std::make_shared<Mummy>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::Larson)
    {
      object = std::make_shared<Larson>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::CrocodileOnLand || item.type == TR1ItemId::CrocodileInWater)
    {
      object = std::make_shared<Crocodile>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::LionMale || item.type == TR1ItemId::LionFemale || item.type == TR1ItemId::Panther)
    {
      object = std::make_shared<Lion>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::Barricade)
    {
      object = std::make_shared<Barricade>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::Gorilla)
    {
      object = std::make_shared<Gorilla>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::Pierre)
    {
      object = std::make_shared<Pierre>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::ThorHammerBlock)
    {
      object = std::make_shared<ThorHammerBlock>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::ThorHammerHandle)
    {
      object = std::make_shared<ThorHammerHandle>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::FlameEmitter)
    {
      object = std::make_shared<FlameEmitter>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::ThorLightningBall)
    {
      object = std::make_shared<LightningBall>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::RatInWater || item.type == TR1ItemId::RatOnLand)
    {
      object = std::make_shared<Rat>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::SlammingDoors)
    {
      object = std::make_shared<SlammingDoors>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::FlyingMutant)
    {
      object = std::make_shared<FlyingMutant>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::WalkingMutant1 || item.type == TR1ItemId::WalkingMutant2)
    {
      // Special handling, these are just "mutations" of the flying mutants (no wings).
      object = std::make_shared<WalkingMutant>(
        &world, room, item, world.findAnimatedModelForType(TR1ItemId::FlyingMutant).get());
    }
    else if(item.type == TR1ItemId::MutantEggSmall || item.type == TR1ItemId::MutantEggBig)
    {
      object = std::make_shared<MutantEgg>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::CentaurMutant)
    {
      object = std::make_shared<CentaurMutant>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::TorsoBoss)
    {
      object = std::make_shared<TorsoBoss>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::LavaParticleEmitter)
    {
      object = std::make_shared<LavaParticleEmitter>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::FlowingAtlanteanLava)
    {
      object = std::make_shared<AtlanteanLava>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::ScionPiece3)
    {
      object = std::make_shared<ScionPiece3>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::ScionPiece4)
    {
      object = std::make_shared<ScionPiece4>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::ScionHolder)
    {
      object = std::make_shared<ScionHolder>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::Earthquake)
    {
      object = std::make_shared<Earthquake>(&world, room, item, model.get());
    }
    else if(item.type == TR1ItemId::Doppelganger)
    {
      object = std::make_shared<Doppelganger>(&world, room, item, model.get());
    }
    else
    {
      const auto stub = std::make_shared<StubObject>(&world, room, item, model.get());
      object = stub;
      if(item.type == TR1ItemId::CameraTarget)
      {
        stub->getSkeleton()->setRenderable(nullptr);
        stub->getSkeleton()->clearParts();
      }
      else
      {
        BOOST_LOG_TRIVIAL(warning) << "Unimplemented object type " << toString(item.type.get_as<TR1ItemId>());
      }
    }

    addChild(room->node, object->getNode());

    object->applyTransform();
    object->updateLighting();

    return object;
  }

  if(const auto& spriteSequence = world.findSpriteSequenceForType(item.type))
  {
    BOOST_ASSERT(!world.findAnimatedModelForType(item.type));
    BOOST_ASSERT(!spriteSequence->sprites.empty());

    const loader::file::Sprite& sprite = spriteSequence->sprites[0];
    std::shared_ptr<Object> object;

    if(item.type == TR1ItemId::ScionPiece1)
    {
      object = std::make_shared<ScionPiece>(&world,
                                            std::string("sprite(type:") + toString(item.type.get_as<TR1ItemId>()) + ")",
                                            room,
                                            item,
                                            sprite,
                                            world.getPresenter().getMaterialManager()->getSprite());
    }
    else if(item.type == TR1ItemId::Item141 || item.type == TR1ItemId::Item142 || item.type == TR1ItemId::Key1Sprite
            || item.type == TR1ItemId::Key2Sprite || item.type == TR1ItemId::Key3Sprite
            || item.type == TR1ItemId::Key4Sprite || item.type == TR1ItemId::Puzzle1Sprite
            || item.type == TR1ItemId::Puzzle2Sprite || item.type == TR1ItemId::Puzzle3Sprite
            || item.type == TR1ItemId::Puzzle4Sprite || item.type == TR1ItemId::PistolsSprite
            || item.type == TR1ItemId::ShotgunSprite || item.type == TR1ItemId::MagnumsSprite
            || item.type == TR1ItemId::UzisSprite || item.type == TR1ItemId::PistolAmmoSprite
            || item.type == TR1ItemId::ShotgunAmmoSprite || item.type == TR1ItemId::MagnumAmmoSprite
            || item.type == TR1ItemId::UziAmmoSprite || item.type == TR1ItemId::ExplosiveSprite
            || item.type == TR1ItemId::SmallMedipackSprite || item.type == TR1ItemId::LargeMedipackSprite
            || item.type == TR1ItemId::ScionPiece2 || item.type == TR1ItemId::LeadBarSprite)
    {
      object
        = std::make_shared<PickupObject>(&world,
                                         std::string("sprite(type:") + toString(item.type.get_as<TR1ItemId>()) + ")",
                                         room,
                                         item,
                                         &sprite,
                                         world.getPresenter().getMaterialManager()->getSprite());
    }
    else
    {
      BOOST_LOG_TRIVIAL(warning) << "Unimplemented object type " << toString(item.type.get_as<TR1ItemId>());
      object
        = std::make_shared<SpriteObject>(&world,
                                         std::string("sprite(type:") + toString(item.type.get_as<TR1ItemId>()) + ")",
                                         room,
                                         item,
                                         true,
                                         &sprite,
                                         world.getPresenter().getMaterialManager()->getSprite());
    }

    return object;
  }

  BOOST_LOG_TRIVIAL(error) << "Failed to find an appropriate animated model for object type " << int(item.type.get());
  return nullptr;
}

gsl::not_null<std::shared_ptr<Object>> create(const serialization::TypeId<gsl::not_null<std::shared_ptr<Object>>>&,
                                              const serialization::Serializer& ser)
{
  const auto type = core::TypeId::create(ser["@type"]);
  const auto position = core::RoomBoundPosition::create(ser["@position"]);
  std::shared_ptr<Object> object;
  std::string spriteName;
#define CREATE(ENUM, TYPE)                                 \
  case TR1ItemId::ENUM:                                    \
    object = std::make_shared<TYPE>(&ser.world, position); \
    object->serialize(ser);                                \
    return object
#define CREATE_SPRITE(ENUM, TYPE)                                                                               \
  case TR1ItemId::ENUM:                                                                                         \
    ser(S_NV("@name", spriteName));                                                                             \
    object = std::make_shared<TYPE>(                                                                            \
      &ser.world, position, std::move(spriteName), ser.world.getPresenter().getMaterialManager()->getSprite()); \
    object->serialize(ser);                                                                                     \
    return object
#define CREATE_PU(ENUM) CREATE_SPRITE(ENUM, PickupObject)
#define CREATE_ID(NAME) CREATE(NAME, NAME)

  switch(type.get_as<TR1ItemId>())
  {
    CREATE(Lara, LaraObject);
    CREATE_ID(Wolf);
    CREATE_ID(Bear);
    CREATE_ID(Bat);
    CREATE(FallingBlock, CollapsibleFloor);
    CREATE_ID(SwingingBlade);
    CREATE_ID(RollingBall);
    CREATE_ID(Dart);
    CREATE(DartEmitter, DartGun);
    CREATE(LiftingDoor, TrapDoorUp);
    CREATE(PushableBlock1, Block);
    CREATE(PushableBlock2, Block);
    CREATE(PushableBlock3, Block);
    CREATE(PushableBlock4, Block);
    CREATE(MovingBlock, TallBlock);
    CREATE(WallSwitch, Switch);
    CREATE_ID(UnderwaterSwitch);
    CREATE(Door1, Door);
    CREATE(Door2, Door);
    CREATE(Door3, Door);
    CREATE(Door4, Door);
    CREATE(Door5, Door);
    CREATE(Door6, Door);
    CREATE(Door7, Door);
    CREATE(Door8, Door);
    CREATE(Trapdoor1, TrapDoorDown);
    CREATE(Trapdoor2, TrapDoorDown);
    CREATE_ID(BridgeFlat);
    CREATE(BridgeTilt1, BridgeSlope1);
    CREATE(BridgeTilt2, BridgeSlope2);
    CREATE(Keyhole1, KeyHole);
    CREATE(Keyhole2, KeyHole);
    CREATE(Keyhole3, KeyHole);
    CREATE(Keyhole4, KeyHole);
    CREATE(PuzzleHole1, PuzzleHole);
    CREATE(PuzzleHole2, PuzzleHole);
    CREATE(PuzzleHole3, PuzzleHole);
    CREATE(PuzzleHole4, PuzzleHole);
    CREATE(Animating1, Animating);
    CREATE(Animating2, Animating);
    CREATE(Animating3, Animating);
    CREATE_ID(TeethSpikes);
    CREATE_ID(Raptor);
    CREATE_ID(SwordOfDamocles);
    CREATE(FallingCeiling, SwordOfDamocles);
    CREATE_ID(CutsceneActor1);
    CREATE_ID(CutsceneActor2);
    CREATE_ID(CutsceneActor3);
    CREATE_ID(CutsceneActor4);
    CREATE_ID(WaterfallMist);
    CREATE_ID(TRex);
    CREATE_ID(Mummy);
    CREATE_ID(Larson);
    CREATE(CrocodileOnLand, Crocodile);
    CREATE(CrocodileInWater, Crocodile);
    CREATE(LionMale, Lion);
    CREATE(LionFemale, Lion);
    CREATE(Panther, Lion);
    CREATE_ID(Barricade);
    CREATE_ID(Gorilla);
    CREATE_ID(Pierre);
    CREATE_ID(ThorHammerBlock);
    CREATE_ID(ThorHammerHandle);
    CREATE_ID(FlameEmitter);
    CREATE(ThorLightningBall, LightningBall);
    CREATE(RatInWater, Rat);
    CREATE(RatOnLand, Rat);
    CREATE_ID(SlammingDoors);
    CREATE_ID(FlyingMutant);
    CREATE(WalkingMutant1, WalkingMutant);
    CREATE(WalkingMutant2, WalkingMutant);
    CREATE(MutantEggSmall, MutantEgg);
    CREATE(MutantEggBig, MutantEgg);
    CREATE_ID(CentaurMutant);
    CREATE_ID(TorsoBoss);
    CREATE_ID(LavaParticleEmitter);
    CREATE(FlowingAtlanteanLava, AtlanteanLava);
    CREATE_ID(ScionPiece3);
    CREATE_ID(ScionPiece4);
    CREATE_ID(ScionHolder);
    CREATE_PU(Item141);
    CREATE_PU(Item142);
    CREATE_PU(Key1Sprite);
    CREATE_PU(Key2Sprite);
    CREATE_PU(Key3Sprite);
    CREATE_PU(Key4Sprite);
    CREATE_PU(Puzzle1Sprite);
    CREATE_PU(Puzzle2Sprite);
    CREATE_PU(Puzzle3Sprite);
    CREATE_PU(Puzzle4Sprite);
    CREATE_PU(PistolsSprite);
    CREATE_PU(ShotgunSprite);
    CREATE_PU(MagnumsSprite);
    CREATE_PU(UzisSprite);
    CREATE_PU(PistolAmmoSprite);
    CREATE_PU(ShotgunAmmoSprite);
    CREATE_PU(MagnumAmmoSprite);
    CREATE_PU(UziAmmoSprite);
    CREATE_PU(ExplosiveSprite);
    CREATE_PU(SmallMedipackSprite);
    CREATE_PU(LargeMedipackSprite);
    CREATE_PU(ScionPiece2);
    CREATE_PU(LeadBarSprite);
    CREATE(SavegameCrystal, StubObject);
    CREATE_ID(Doppelganger);
    CREATE_ID(Earthquake);
    CREATE_SPRITE(ScionPiece1, ScionPiece);
  case TR1ItemId::CameraTarget:
  {
    const auto stub = std::make_shared<StubObject>(&ser.world, position);
    stub->serialize(ser);
    stub->getSkeleton()->setRenderable(nullptr);
    stub->getSkeleton()->clearParts();
    return stub;
  }
  default: BOOST_THROW_EXCEPTION(std::domain_error("Cannot create unknown object type " + std::to_string(type.get())));
  }
#undef CREATE
#undef CREATE_ID
#undef CREATE_PU
}
} // namespace engine::objects

void serialization::serialize(std::shared_ptr<engine::objects::Object>& ptr, const Serializer& ser)
{
  Expects(!ser.loading);
  Expects(ptr != nullptr);
  ptr->serialize(ser);
}
