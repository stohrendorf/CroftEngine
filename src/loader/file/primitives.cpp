#include "primitives.h"

#include "io/sdlreader.h"

namespace loader::file
{
Triangle Triangle::read(io::SDLReader& reader, const bool withLighting)
{
  Triangle triangle;
  triangle.vertices[0] = reader.readU16();
  triangle.vertices[1] = reader.readU16();
  triangle.vertices[2] = reader.readU16();
  triangle.tileId = reader.readU16();
  if(withLighting)
    triangle.lighting = reader.readU16();
  else
    triangle.lighting = 0;
  return triangle;
}

QuadFace QuadFace::read(io::SDLReader& reader, const bool withLighting)
{
  QuadFace quadFace;
  quadFace.vertices[0] = reader.readU16();
  quadFace.vertices[1] = reader.readU16();
  quadFace.vertices[2] = reader.readU16();
  quadFace.vertices[3] = reader.readU16();
  quadFace.tileId = reader.readU16();
  if(withLighting)
    quadFace.lighting = reader.readU16();
  else
    quadFace.lighting = 0;
  return quadFace;
}
} // namespace loader::file