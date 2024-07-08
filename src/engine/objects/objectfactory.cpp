#include "objectfactory.h"

#include "animating.h"
#include "atlanteanlava.h"
#include "barricade.h"
#include "bat.h"
#include "bear.h"
#include "block.h"
#include "bridgeflat.h"
#include "centaurstatue.h"
#include "collapsiblefloor.h"
#include "core/id.h"
#include "cowboy.h"
#include "crocodile.h"
#include "cutsceneactors.h"
#include "dart.h"
#include "dartgun.h"
#include "door.h"
#include "doppelganger.h"
#include "earthquake.h"
#include "engine/engine.h"
#include "engine/items_tr1.h"
#include "engine/location.h"
#include "engine/script/reflection.h"
#include "engine/skeletalmodelnode.h"
#include "engine/world/room.h"
#include "engine/world/sprite.h"
#include "engine/world/world.h"
#include "fallingceiling.h"
#include "flameemitter.h"
#include "gorilla.h"
#include "keyhole.h"
#include "kold.h"
#include "laraobject.h"
#include "larson.h"
#include "lavaparticleemitter.h"
#include "lightningemitter.h"
#include "lion.h"
#include "loader/file/item.h"
#include "midasgoldtouch.h"
#include "motorboat.h"
#include "mummy.h"
#include "mutant.h"
#include "mutantegg.h"
#include "natla.h"
#include "object.h"
#include "pickupobject.h"
#include "pierre.h"
#include "puzzlehole.h"
#include "raptor.h"
#include "rat.h"
#include "render/scene/node.h"
#include "rollingball.h"
#include "scionpiece.h"
#include "serialization/serialization.h"
#include "skateboardkid.h"
#include "slammingdoors.h"
#include "slopedbridge.h"
#include "spriteobject.h"
#include "stubobject.h"
#include "suspendedshack.h"
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

#include <boost/assert.hpp>
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <cstddef>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace engine::objects
{
namespace
{
/* NOLINTNEXTLINE(altera-struct-pack-align) */
struct ObjectFactory
{
  virtual ~ObjectFactory() = default;

  [[nodiscard]] virtual gslu::nn_shared<Object>
    createNew(world::World& world, loader::file::Item& item, size_t id) const = 0;
  [[nodiscard]] virtual gslu::nn_shared<Object>
    createFromSave(const Location& location, const serialization::Deserializer<world::World>& ser) const = 0;
};

/* NOLINTNEXTLINE(altera-struct-pack-align) */
struct UnsupportedObjectFactory : public ObjectFactory
{
  ~UnsupportedObjectFactory() override = default;

  [[nodiscard]] [[noreturn]] gslu::nn_shared<Object>
    createNew(world::World& /*world*/, loader::file::Item& item, size_t /*id*/) const override
  {
    BOOST_LOG_TRIVIAL(fatal) << "Object type " << toString(item.type.get_as<engine::TR1ItemId>())
                             << " is not supported";
    BOOST_THROW_EXCEPTION(std::runtime_error("unsupported object type"));
  }

  [[nodiscard]] [[noreturn]] gslu::nn_shared<Object>
    createFromSave(const Location& /*location*/,
                   const serialization::Deserializer<world::World>& /*ser*/) const override
  {
    BOOST_LOG_TRIVIAL(fatal) << "Object type is not supported";
    BOOST_THROW_EXCEPTION(std::runtime_error("unsupported object type"));
  }
};

template<typename T>
/* NOLINTNEXTLINE(altera-struct-pack-align) */
struct ModelFactory : public ObjectFactory
{
  [[nodiscard]] gslu::nn_shared<Object>
    createNew(world::World& world, loader::file::Item& item, size_t id) const override
  {
    const auto room = gsl::not_null{&world.getRooms().at(item.room.get())};
    const auto& model = world.getWorldGeometry().findAnimatedModelForType(item.type);
    gsl_Assert(model != nullptr);
    auto object = gsl::make_shared<T>(
      makeObjectName(item.type.get_as<TR1ItemId>(), id), gsl::not_null{&world}, room, item, gsl::not_null{model.get()});
    addChild(gsl::not_null{room->node}, gsl::not_null{object->getNode()});

    world.getEngine()
      .getScriptEngine()
      .getGameflow()
      .getObjectInfos()
      .at(item.type.get_as<TR1ItemId>())
      ->customize(object);

    object->applyTransform();
    return object;
  }

  [[nodiscard]] gslu::nn_shared<Object>
    createFromSave(const Location& location, const serialization::Deserializer<world::World>& ser) const override
  {
    auto object = gsl::make_shared<T>(ser.context, location);
    object->deserialize(ser);
    return object;
  }
};

template<typename T>
/* NOLINTNEXTLINE(altera-struct-pack-align) */
struct SpriteFactory : public ObjectFactory
{
  [[nodiscard]] gslu::nn_shared<Object>
    createNew(world::World& world, loader::file::Item& item, size_t id) const override
  {
    const auto room = gsl::not_null{&world.getRooms().at(item.room.get())};

    const auto& spriteSequence = world.getWorldGeometry().findSpriteSequenceForType(item.type);
    gsl_Assert(spriteSequence != nullptr && !spriteSequence->sprites.empty());

    const world::Sprite& sprite = spriteSequence->sprites[0];
    auto object = gsl::make_shared<T>(
      makeObjectName(item.type.get_as<TR1ItemId>(), id), gsl::not_null{&world}, room, item, gsl::not_null{&sprite});

    world.getEngine()
      .getScriptEngine()
      .getGameflow()
      .getObjectInfos()
      .at(item.type.get_as<TR1ItemId>())
      ->customize(object);

    return object;
  }

  [[nodiscard]] gslu::nn_shared<Object>
    createFromSave(const Location& location, const serialization::Deserializer<world::World>& ser) const override
  {
    std::string spriteName;
    ser(S_NV("@name", spriteName));
    auto object = gsl::make_shared<T>(spriteName, ser.context, location);
    object->deserialize(ser);
    return object;
  }
};

#define _PAREN_WRAPPER(ARG) ARG

#define MODEL_FACTORY(ENUM, CLASS)                                                           \
  {                                                                                          \
    TR1ItemId::_PAREN_WRAPPER(ENUM), std::make_shared<ModelFactory<_PAREN_WRAPPER(CLASS)>>() \
  }

#define SPRITE_FACTORY(ENUM, CLASS)                                                           \
  {                                                                                           \
    TR1ItemId::_PAREN_WRAPPER(ENUM), std::make_shared<SpriteFactory<_PAREN_WRAPPER(CLASS)>>() \
  }

#define UNSUPPORTED_FACTORY(ENUM)                                                 \
  {                                                                               \
    TR1ItemId::_PAREN_WRAPPER(ENUM), std::make_shared<UnsupportedObjectFactory>() \
  }

/* NOLINTNEXTLINE(altera-struct-pack-align) */
struct WalkingMutantFactory : public ObjectFactory
{
  [[nodiscard]] gslu::nn_shared<Object>
    createNew(world::World& world, loader::file::Item& item, size_t id) const override
  {
    const auto room = gsl::not_null{&world.getRooms().at(item.room.get())};
    const auto& model = world.getWorldGeometry().findAnimatedModelForType(TR1ItemId::FlyingMutant);
    gsl_Assert(model != nullptr);
    auto object = gsl::make_shared<WalkingMutant>(
      makeObjectName(item.type.get_as<TR1ItemId>(), id), gsl::not_null{&world}, room, item, gsl::not_null{model.get()});
    addChild(gsl::not_null{room->node}, gsl::not_null{object->getNode()});

    world.getEngine()
      .getScriptEngine()
      .getGameflow()
      .getObjectInfos()
      .at(item.type.get_as<TR1ItemId>())
      ->customize(object);

    object->applyTransform();
    return object;
  }

  [[nodiscard]] gslu::nn_shared<Object>
    createFromSave(const Location& location, const serialization::Deserializer<world::World>& ser) const override
  {
    auto object = gsl::make_shared<WalkingMutant>(ser.context, location);
    object->deserialize(ser);
    return object;
  }
};

/* NOLINTNEXTLINE(altera-struct-pack-align) */
struct HiddenModelFactory : public ModelFactory<StubObject>
{
  [[nodiscard]] gslu::nn_shared<Object>
    createNew(world::World& world, loader::file::Item& item, size_t id) const override
  {
    auto object = gslu::static_pointer_cast<StubObject>(ModelFactory<StubObject>::createNew(world, item, id));
    object->getSkeleton()->setRenderable(nullptr);
    object->getSkeleton()->clearParts();

    world.getEngine()
      .getScriptEngine()
      .getGameflow()
      .getObjectInfos()
      .at(item.type.get_as<TR1ItemId>())
      ->customize(object);

    return object;
  }

  [[nodiscard]] gslu::nn_shared<Object>
    createFromSave(const Location& location, const serialization::Deserializer<world::World>& ser) const override
  {
    auto object = gslu::static_pointer_cast<StubObject>(ModelFactory<StubObject>::createFromSave(location, ser));
    object->getSkeleton()->setRenderable(nullptr);
    object->getSkeleton()->clearParts();
    return object;
  }
};

const auto& getFactories()
{
  static const std::map<TR1ItemId, std::shared_ptr<ObjectFactory>> factories{
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
    MODEL_FACTORY(PuzzleDone1, PuzzleHole),
    MODEL_FACTORY(PuzzleDone2, PuzzleHole),
    MODEL_FACTORY(PuzzleDone3, PuzzleHole),
    MODEL_FACTORY(PuzzleDone4, PuzzleHole),
    MODEL_FACTORY(Animating1, Animating),
    MODEL_FACTORY(Animating2, Animating),
    MODEL_FACTORY(Animating3, Animating),
    MODEL_FACTORY(TeethSpikes, TeethSpikes),
    MODEL_FACTORY(Raptor, Raptor),
    MODEL_FACTORY(SwordOfDamocles, SwordOfDamocles),
    MODEL_FACTORY(FallingCeiling, FallingCeiling),
    MODEL_FACTORY(FallingCeiling2, FallingCeiling),
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
    MODEL_FACTORY(LightningEmitter, LightningEmitter),
    MODEL_FACTORY(RatInWater, Rat),
    MODEL_FACTORY(RatOnLand, Rat),
    MODEL_FACTORY(SlammingDoors, SlammingDoors),
    MODEL_FACTORY(FlyingMutant, FlyingMutant),
    MODEL_FACTORY(CentaurStatue, CentaurStatue),
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
    MODEL_FACTORY(MidasGoldTouch, MidasGoldTouch),
    MODEL_FACTORY(Motorboat, Motorboat),
    MODEL_FACTORY(SuspendedShack, SuspendedShack),
    MODEL_FACTORY(Kold, Kold),
    MODEL_FACTORY(Cowboy, Cowboy),
    MODEL_FACTORY(Natla, Natla),
    {TR1ItemId::WalkingMutant1, std::make_unique<WalkingMutantFactory>()},
    {TR1ItemId::WalkingMutant2, std::make_unique<WalkingMutantFactory>()},
    {TR1ItemId::CameraTarget, std::make_unique<HiddenModelFactory>()},
    {TR1ItemId::SavegameCrystal, std::make_unique<HiddenModelFactory>()},
    MODEL_FACTORY(SkateboardKid, SkateboardKid),
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
    {TR1ItemId::Sunglasses, std::make_unique<HiddenModelFactory>()},
    UNSUPPORTED_FACTORY(DinoWarrior),
    UNSUPPORTED_FACTORY(Fish),
    UNSUPPORTED_FACTORY(Skateboard),
  };
  return factories;
}

ObjectFactory* findFactory(TR1ItemId type)
{
  const auto& factories = getFactories();

  if(auto it = factories.find(type); it != factories.end())
    return it->second.get();

  return nullptr;
}
} // namespace

std::shared_ptr<Object> createObject(world::World& world, loader::file::Item& item, size_t id)
{
  if(const auto factory = findFactory(item.type.get_as<TR1ItemId>()))
    return factory->createNew(world, item, id);

  const auto room = gsl::not_null{&world.getRooms().at(item.room.get())};

  if(const auto& model = world.getWorldGeometry().findAnimatedModelForType(item.type))
  {
    auto object = std::make_shared<StubObject>(
      makeObjectName(item.type.get_as<TR1ItemId>(), id), gsl::not_null{&world}, room, item, gsl::not_null{model.get()});
    BOOST_LOG_TRIVIAL(warning) << "Unimplemented object type " << toString(item.type.get_as<TR1ItemId>());

    addChild(gsl::not_null{room->node}, gsl::not_null{object->getNode()});

    object->applyTransform();

    return object;
  }

  if(const auto& spriteSequence = world.getWorldGeometry().findSpriteSequenceForType(item.type))
  {
    BOOST_ASSERT(!spriteSequence->sprites.empty());

    const world::Sprite& sprite = spriteSequence->sprites[0];

    BOOST_LOG_TRIVIAL(warning) << "Unimplemented object type " << toString(item.type.get_as<TR1ItemId>());
    return std::make_shared<SpriteObject>(makeObjectName(item.type.get_as<TR1ItemId>(), id),
                                          gsl::not_null{&world},
                                          room,
                                          item,
                                          true,
                                          gsl::not_null{&sprite},
                                          false);
  }

  BOOST_LOG_TRIVIAL(error) << "Failed to find an appropriate animated model for object type " << int(item.type.get());
  return nullptr;
}

gslu::nn_shared<Object> create(const serialization::TypeId<gslu::nn_shared<Object>>&,
                               const serialization::Deserializer<world::World>& ser)
{
  const auto type = core::TypeId::create(ser["@type"]);
  const auto position = Location::create(ser["@location"]);

  if(const auto factory = findFactory(type.get_as<TR1ItemId>()))
    return factory->createFromSave(position, ser);

  BOOST_THROW_EXCEPTION(std::domain_error("Cannot create unknown object type " + std::to_string(type.get())));
}
} // namespace engine::objects

void serialization::serialize(const std::shared_ptr<engine::objects::Object>& ptr,
                              const Serializer<engine::world::World>& ser)
{
  gsl_Expects(ptr != nullptr);
  ptr->serialize(ser);
}

void serialization::deserialize(std::shared_ptr<engine::objects::Object>& ptr,
                                const Deserializer<engine::world::World>& ser)
{
  gsl_Expects(ptr != nullptr);
  ptr->deserialize(ser);
}
