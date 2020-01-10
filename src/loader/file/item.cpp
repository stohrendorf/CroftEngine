#include "item.h"

#include "io/sdlreader.h"
#include "io/util.h"

namespace loader::file
{
std::unique_ptr<Item> Item::readTr1(io::SDLReader& reader)
{
  std::unique_ptr<Item> item{std::make_unique<Item>()};
  item->type = reader.readU16();
  item->room = reader.readU16();
  item->position = readCoordinates32(reader);
  item->rotation = core::auToAngle(reader.readI16());
  item->shade = core::Shade{reader.readI16()};
  item->activationState = reader.readU16();
  return item;
}

std::unique_ptr<Item> Item::readTr2(io::SDLReader& reader)
{
  std::unique_ptr<Item> item{std::make_unique<Item>()};
  item->type = reader.readU16();
  item->room = reader.readU16();
  item->position = readCoordinates32(reader);
  item->rotation = core::auToAngle(reader.readI16());
  item->shade = core::Shade{reader.readI16()};
  item->intensity2 = reader.readU16();
  item->activationState = reader.readU16();
  return item;
}

std::unique_ptr<Item> Item::readTr3(io::SDLReader& reader)
{
  std::unique_ptr<Item> item{std::make_unique<Item>()};
  item->type = reader.readU16();
  item->room = reader.readU16();
  item->position = readCoordinates32(reader);
  item->rotation = core::auToAngle(reader.readI16());
  item->shade = core::Shade{reader.readI16()};
  item->intensity2 = reader.readU16();
  item->activationState = reader.readU16();
  return item;
}

std::unique_ptr<Item> Item::readTr4(io::SDLReader& reader)
{
  std::unique_ptr<Item> item{std::make_unique<Item>()};
  item->type = reader.readU16();
  item->room = reader.readU16();
  item->position = readCoordinates32(reader);
  item->rotation = core::auToAngle(reader.readI16());
  item->shade = core::Shade{reader.readI16()};
  item->intensity2 = item->shade.get();
  item->ocb = reader.readU16();
  item->activationState = reader.readU16();
  return item;
}
} // namespace loader::file
