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

namespace engine::objects
{
namespace
{
/* NOLINTNEXTLINE(altera-struct-pack-align) */
struct ObjectFactory
{
  virtual ~ObjectFactory() = default;

  [[nodiscard]] virtual std::shared_ptr<Object> createNew(World& world, loader::file::Item& item) const = 0;
  [[nodiscard]] virtual std::shared_ptr<Object> createFromSave(const core::RoomBoundPosition& position,
                                                               const serialization::Serializer& ser) const = 0;
};

#define CREATE_MODEL_FACTORY_STRUCT(CLASS)                                                                    \
  /* NOLINTNEXTLINE(altera-struct-pack-align) */                                                              \
  struct Factory : public ObjectFactory                                                                       \
  {                                                                                                           \
    [[nodiscard]] std::shared_ptr<Object> createNew(World& world, loader::file::Item& item) const override    \
    {                                                                                                         \
      const auto* room = &world.getRooms().at(item.room.get());                                               \
      const auto& model = world.findAnimatedModelForType(item.type);                                          \
      Expects(model != nullptr);                                                                              \
      auto object = std::make_shared<CLASS>(&world, room, item, model.get());                                 \
      addChild(room->node, object->getNode());                                                                \
      object->applyTransform();                                                                               \
      object->updateLighting();                                                                               \
      return object;                                                                                          \
    }                                                                                                         \
                                                                                                              \
    [[nodiscard]] std::shared_ptr<Object> createFromSave(const core::RoomBoundPosition& position,             \
                                                         const serialization::Serializer& ser) const override \
    {                                                                                                         \
      auto object = std::make_shared<CLASS>(&ser.world, position);                                            \
      object->serialize(ser);                                                                                 \
      return object;                                                                                          \
    }                                                                                                         \
  }

#define CREATE_SPRITE_FACTORY_STRUCT(CLASS)                                                                       \
  /* NOLINTNEXTLINE(altera-struct-pack-align) */                                                                  \
  struct Factory : public ObjectFactory                                                                           \
  {                                                                                                               \
    [[nodiscard]] std::shared_ptr<Object> createNew(World& world, loader::file::Item& item) const override        \
    {                                                                                                             \
      const auto* room = &world.getRooms().at(item.room.get());                                                   \
                                                                                                                  \
      const auto& spriteSequence = world.findSpriteSequenceForType(item.type);                                    \
      Expects(spriteSequence != nullptr);                                                                         \
      Expects(!spriteSequence->sprites.empty());                                                                  \
                                                                                                                  \
      const loader::file::Sprite& sprite = spriteSequence->sprites[0];                                            \
      return std::make_shared<CLASS>(&world,                                                                      \
                                     std::string("sprite(type:") + toString(item.type.get_as<TR1ItemId>()) + ")", \
                                     room,                                                                        \
                                     item,                                                                        \
                                     &sprite,                                                                     \
                                     world.getPresenter().getMaterialManager()->getSprite());                     \
    }                                                                                                             \
                                                                                                                  \
    [[nodiscard]] std::shared_ptr<Object> createFromSave(const core::RoomBoundPosition& position,                 \
                                                         const serialization::Serializer& ser) const override     \
    {                                                                                                             \
      std::string spriteName;                                                                                     \
      ser(S_NV("@name", spriteName));                                                                             \
      auto object = std::make_shared<CLASS>(                                                                      \
        &ser.world, position, std::move(spriteName), ser.world.getPresenter().getMaterialManager()->getSprite()); \
      object->serialize(ser);                                                                                     \
      return object;                                                                                              \
    }                                                                                                             \
  }

#define CREATE_MODEL_FACTORY(CLASS)         \
  ([]() -> std::unique_ptr<ObjectFactory> { \
    CREATE_MODEL_FACTORY_STRUCT(CLASS);     \
    return std::make_unique<Factory>();     \
  })()

#define CREATE_SPRITE_FACTORY(CLASS)        \
  ([]() -> std::unique_ptr<ObjectFactory> { \
    CREATE_SPRITE_FACTORY_STRUCT(CLASS);    \
    return std::make_unique<Factory>();     \
  })()

#define MODEL_FACTORY(ENUM, CLASS)               \
  std::pair                                      \
  {                                              \
    TR1ItemId::ENUM, CREATE_MODEL_FACTORY(CLASS) \
  }

#define SPRITE_FACTORY(ENUM, CLASS)               \
  std::pair                                       \
  {                                               \
    TR1ItemId::ENUM, CREATE_SPRITE_FACTORY(CLASS) \
  }

/* NOLINTNEXTLINE(altera-struct-pack-align) */
struct WalkingMutantFactory : public ObjectFactory
{
  [[nodiscard]] std::shared_ptr<Object> createNew(World& world, loader::file::Item& item) const override
  {
    const auto* room = &world.getRooms().at(item.room.get());
    const auto& model = world.findAnimatedModelForType(item.type);
    Expects(model != nullptr);
    auto object = std::make_shared<WalkingMutant>(&world, room, item, model.get());
    addChild(room->node, object->getNode());
    object->applyTransform();
    object->updateLighting();
    return object;
  }

  [[nodiscard]] std::shared_ptr<Object> createFromSave(const core::RoomBoundPosition& position,
                                                       const serialization::Serializer& ser) const override
  {
    auto object = std::make_shared<WalkingMutant>(&ser.world, position);
    object->serialize(ser);
    return object;
  }
};

/* NOLINTNEXTLINE(altera-struct-pack-align) */
struct CameraTargetFactory : public ObjectFactory
{
  [[nodiscard]] std::shared_ptr<Object> createNew(World& world, loader::file::Item& item) const override
  {
    const auto* room = &world.getRooms().at(item.room.get());
    const auto& model = world.findAnimatedModelForType(item.type);
    Expects(model != nullptr);
    auto object = std::make_shared<StubObject>(&world, room, item, model.get());
    addChild(room->node, object->getNode());
    object->applyTransform();
    object->updateLighting();
    object->getSkeleton()->setRenderable(nullptr);
    object->getSkeleton()->clearParts();
    return object;
  }

  [[nodiscard]] std::shared_ptr<Object> createFromSave(const core::RoomBoundPosition& position,
                                                       const serialization::Serializer& ser) const override
  {
    auto object = std::make_shared<StubObject>(&ser.world, position);
    object->serialize(ser);
    object->getSkeleton()->setRenderable(nullptr);
    object->getSkeleton()->clearParts();
    return object;
  }
};

/* NOLINTNEXTLINE(altera-struct-pack-align) */
struct SavegameCrystalFactory : public ObjectFactory
{
  [[nodiscard]] std::shared_ptr<Object> createNew(World& world, loader::file::Item& item) const override
  {
    const auto* room = &world.getRooms().at(item.room.get());
    const auto& model = world.findAnimatedModelForType(item.type);
    Expects(model != nullptr);
    auto object = std::make_shared<StubObject>(&world, room, item, model.get());
    addChild(room->node, object->getNode());
    object->applyTransform();
    object->updateLighting();
    object->getSkeleton()->setRenderable(nullptr);
    object->getSkeleton()->clearParts();
    return object;
  }

  [[nodiscard]] std::shared_ptr<Object> createFromSave(const core::RoomBoundPosition& position,
                                                       const serialization::Serializer& ser) const override
  {
    auto object = std::make_shared<StubObject>(&ser.world, position);
    object->serialize(ser);
    object->getSkeleton()->setRenderable(nullptr);
    object->getSkeleton()->clearParts();
    return object;
  }
};

const std::pair<TR1ItemId, std::unique_ptr<ObjectFactory>> factories[]{
  MODEL_FACTORY(Lara, LaraObject),
  MODEL_FACTORY(Wolf, Wolf),
  MODEL_FACTORY(Bear, Bear),
  MODEL_FACTORY(FallingBlock, CollapsibleFloor),
  MODEL_FACTORY(SwingingBlade, SwingingBlade),
  MODEL_FACTORY(RollingBall, RollingBall),
  MODEL_FACTORY(Dart, Dart),
  MODEL_FACTORY(Bat, Bat),
  MODEL_FACTORY(DartEmitter, DartGun),
  MODEL_FACTORY(LiftingDoor, TrapDoorUp),
  MODEL_FACTORY(PushableBlock1, Block),
  MODEL_FACTORY(PushableBlock2, Block),
  MODEL_FACTORY(PushableBlock3, Block),
  MODEL_FACTORY(PushableBlock4, Block),
  MODEL_FACTORY(MovingBlock, TallBlock),
  MODEL_FACTORY(WallSwitch, Switch),
  MODEL_FACTORY(UnderwaterSwitch, UnderwaterSwitch),
  MODEL_FACTORY(Door1, Door),
  MODEL_FACTORY(Door2, Door),
  MODEL_FACTORY(Door3, Door),
  MODEL_FACTORY(Door4, Door),
  MODEL_FACTORY(Door5, Door),
  MODEL_FACTORY(Door6, Door),
  MODEL_FACTORY(Door7, Door),
  MODEL_FACTORY(Door8, Door),
  MODEL_FACTORY(Trapdoor1, TrapDoorDown),
  MODEL_FACTORY(Trapdoor2, TrapDoorDown),
  MODEL_FACTORY(BridgeFlat, BridgeFlat),
  MODEL_FACTORY(BridgeTilt1, BridgeSlope1),
  MODEL_FACTORY(BridgeTilt2, BridgeSlope2),
  MODEL_FACTORY(Keyhole1, KeyHole),
  MODEL_FACTORY(Keyhole2, KeyHole),
  MODEL_FACTORY(Keyhole3, KeyHole),
  MODEL_FACTORY(Keyhole4, KeyHole),
  MODEL_FACTORY(PuzzleHole1, PuzzleHole),
  MODEL_FACTORY(PuzzleHole2, PuzzleHole),
  MODEL_FACTORY(PuzzleHole3, PuzzleHole),
  MODEL_FACTORY(PuzzleHole4, PuzzleHole),
  MODEL_FACTORY(Animating1, Animating),
  MODEL_FACTORY(Animating2, Animating),
  MODEL_FACTORY(Animating3, Animating),
  MODEL_FACTORY(TeethSpikes, TeethSpikes),
  MODEL_FACTORY(Raptor, Raptor),
  MODEL_FACTORY(SwordOfDamocles, SwordOfDamocles),
  MODEL_FACTORY(FallingCeiling, SwordOfDamocles),
  MODEL_FACTORY(CutsceneActor1, CutsceneActor1),
  MODEL_FACTORY(CutsceneActor2, CutsceneActor2),
  MODEL_FACTORY(CutsceneActor3, CutsceneActor3),
  MODEL_FACTORY(CutsceneActor4, CutsceneActor4),
  MODEL_FACTORY(WaterfallMist, WaterfallMist),
  MODEL_FACTORY(TRex, TRex),
  MODEL_FACTORY(Mummy, Mummy),
  MODEL_FACTORY(Larson, Larson),
  MODEL_FACTORY(CrocodileOnLand, Crocodile),
  MODEL_FACTORY(CrocodileInWater, Crocodile),
  MODEL_FACTORY(LionMale, Lion),
  MODEL_FACTORY(LionFemale, Lion),
  MODEL_FACTORY(Panther, Lion),
  MODEL_FACTORY(Barricade, Barricade),
  MODEL_FACTORY(Gorilla, Gorilla),
  MODEL_FACTORY(Pierre, Pierre),
  MODEL_FACTORY(ThorHammerBlock, ThorHammerBlock),
  MODEL_FACTORY(ThorHammerHandle, ThorHammerHandle),
  MODEL_FACTORY(FlameEmitter, FlameEmitter),
  MODEL_FACTORY(ThorLightningBall, LightningBall),
  MODEL_FACTORY(RatInWater, Rat),
  MODEL_FACTORY(RatOnLand, Rat),
  MODEL_FACTORY(SlammingDoors, SlammingDoors),
  MODEL_FACTORY(FlyingMutant, FlyingMutant),
  MODEL_FACTORY(MutantEggSmall, MutantEgg),
  MODEL_FACTORY(MutantEggBig, MutantEgg),
  MODEL_FACTORY(CentaurMutant, CentaurMutant),
  MODEL_FACTORY(TorsoBoss, TorsoBoss),
  MODEL_FACTORY(LavaParticleEmitter, LavaParticleEmitter),
  MODEL_FACTORY(FlowingAtlanteanLava, AtlanteanLava),
  MODEL_FACTORY(ScionPiece3, ScionPiece3),
  MODEL_FACTORY(ScionPiece4, ScionPiece4),
  MODEL_FACTORY(ScionHolder, ScionHolder),
  MODEL_FACTORY(Earthquake, Earthquake),
  MODEL_FACTORY(Doppelganger, Doppelganger),
  MODEL_FACTORY(LarasHomePolaroid, StubObject),
  std::pair{TR1ItemId::WalkingMutant1,
            ([]() -> std::unique_ptr<ObjectFactory> { return std::make_unique<WalkingMutantFactory>(); })()},
  std::pair{TR1ItemId::WalkingMutant2,
            ([]() -> std::unique_ptr<ObjectFactory> { return std::make_unique<WalkingMutantFactory>(); })()},
  std::pair{TR1ItemId::CameraTarget,
            ([]() -> std::unique_ptr<ObjectFactory> { return std::make_unique<CameraTargetFactory>(); })()},
  std::pair{TR1ItemId::SavegameCrystal,
            ([]() -> std::unique_ptr<ObjectFactory> { return std::make_unique<SavegameCrystalFactory>(); })()},
  SPRITE_FACTORY(ScionPiece1, ScionPiece),
  SPRITE_FACTORY(Item141, PickupObject),
  SPRITE_FACTORY(Item142, PickupObject),
  SPRITE_FACTORY(Key1Sprite, PickupObject),
  SPRITE_FACTORY(Key2Sprite, PickupObject),
  SPRITE_FACTORY(Key3Sprite, PickupObject),
  SPRITE_FACTORY(Key4Sprite, PickupObject),
  SPRITE_FACTORY(Puzzle1Sprite, PickupObject),
  SPRITE_FACTORY(Puzzle2Sprite, PickupObject),
  SPRITE_FACTORY(Puzzle3Sprite, PickupObject),
  SPRITE_FACTORY(Puzzle4Sprite, PickupObject),
  SPRITE_FACTORY(PistolsSprite, PickupObject),
  SPRITE_FACTORY(ShotgunSprite, PickupObject),
  SPRITE_FACTORY(MagnumsSprite, PickupObject),
  SPRITE_FACTORY(UzisSprite, PickupObject),
  SPRITE_FACTORY(PistolAmmoSprite, PickupObject),
  SPRITE_FACTORY(ShotgunAmmoSprite, PickupObject),
  SPRITE_FACTORY(MagnumAmmoSprite, PickupObject),
  SPRITE_FACTORY(UziAmmoSprite, PickupObject),
  SPRITE_FACTORY(ExplosiveSprite, PickupObject),
  SPRITE_FACTORY(SmallMedipackSprite, PickupObject),
  SPRITE_FACTORY(LargeMedipackSprite, PickupObject),
  SPRITE_FACTORY(ScionPiece2, PickupObject),
  SPRITE_FACTORY(LeadBarSprite, PickupObject),
};

ObjectFactory* findFactory(TR1ItemId type)
{
  for(const auto& factory : factories)
    if(factory.first == type)
      return factory.second.get();

  return nullptr;
}
} // namespace

std::shared_ptr<Object> createObject(World& world, loader::file::Item& item)
{
  if(const auto factory = findFactory(item.type.get_as<TR1ItemId>()))
    return factory->createNew(world, item);

  const auto* room = &world.getRooms().at(item.room.get());

  if(const auto& model = world.findAnimatedModelForType(item.type))
  {
    auto object = std::make_shared<StubObject>(&world, room, item, model.get());
    BOOST_LOG_TRIVIAL(warning) << "Unimplemented object type " << toString(item.type.get_as<TR1ItemId>());

    addChild(room->node, object->getNode());

    object->applyTransform();
    object->updateLighting();

    return object;
  }

  if(const auto& spriteSequence = world.findSpriteSequenceForType(item.type))
  {
    BOOST_ASSERT(!spriteSequence->sprites.empty());

    const loader::file::Sprite& sprite = spriteSequence->sprites[0];
    std::shared_ptr<Object> object;

    BOOST_LOG_TRIVIAL(warning) << "Unimplemented object type " << toString(item.type.get_as<TR1ItemId>());
    return std::make_shared<SpriteObject>(&world,
                                          std::string("sprite(type:") + toString(item.type.get_as<TR1ItemId>()) + ")",
                                          room,
                                          item,
                                          true,
                                          &sprite,
                                          world.getPresenter().getMaterialManager()->getSprite());
  }

  BOOST_LOG_TRIVIAL(error) << "Failed to find an appropriate animated model for object type " << int(item.type.get());
  return nullptr;
}

gsl::not_null<std::shared_ptr<Object>> create(const serialization::TypeId<gsl::not_null<std::shared_ptr<Object>>>&,
                                              const serialization::Serializer& ser)
{
  const auto type = core::TypeId::create(ser["@type"]);
  const auto position = core::RoomBoundPosition::create(ser["@position"]);

  if(const auto factory = findFactory(type.get_as<TR1ItemId>()))
    return factory->createFromSave(position, ser);

  BOOST_THROW_EXCEPTION(std::domain_error("Cannot create unknown object type " + std::to_string(type.get())));
}
} // namespace engine::objects

void serialization::serialize(std::shared_ptr<engine::objects::Object>& ptr, const Serializer& ser)
{
  Expects(!ser.loading);
  Expects(ptr != nullptr);
  ptr->serialize(ser);
}
