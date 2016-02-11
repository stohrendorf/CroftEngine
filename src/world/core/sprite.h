#pragma once

#include "render/vertex_array.h"

#include <irrlicht.h>

#include <memory>
#include <vector>

namespace render
{
class VertexArray;
}

namespace world
{
namespace core
{
using SpriteId = uint32_t;
/*
 * base sprite structure
 */
struct Sprite
{
    SpriteId id;
    size_t texture;
    irr::core::vector2df tex_coord[4];
    irr::f32 left;
    irr::f32 right;
    irr::f32 top;
    irr::f32 bottom;
};

/*
 * Structure for all the sprites in a room
 */
struct SpriteBuffer
{
    //! Vertex data for the sprites
    std::unique_ptr<render::VertexArray> data{};

    //! How many sub-ranges the element_array_buffer contains. It has one for each texture listed.
    size_t num_texture_pages = 0;
    //! The element count for each sub-range.
    std::vector<size_t> element_count_per_texture{};
};
} // namespace core
} // namespace world
