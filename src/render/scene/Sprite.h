#pragma once

#include "renderable.h"
#include "mesh.h"

#include "gsl-lite.hpp"

namespace render
{
namespace scene
{
class Material;


class Sprite : public Renderable
{
public:
    enum class Axis
    {
        X, Y, Z
    };

    explicit Sprite(const float x0, const float y0,
                    const float x1, const float y1,
                    const glm::vec2& t0, const glm::vec2& t1,
                    const gsl::not_null<std::shared_ptr<Material>>& material,
                    const Axis pole)
            : m_mesh{createMesh( x0, y0, x1, y1, t0, t1, material, pole )}
    {
    }

    Sprite(const Sprite&) = delete;

    Sprite(Sprite&&) = delete;

    Sprite& operator=(Sprite&&) = delete;

    Sprite& operator=(const Sprite&) = delete;

    ~Sprite() override = default;

    void render(RenderContext& context) override;

private:
    static gsl::not_null<std::shared_ptr<Mesh>> createMesh(float x0,
                                                           float y0,
                                                           float x1,
                                                           float y1,
                                                           const glm::vec2& t0,
                                                           const glm::vec2& t1,
                                                           const gsl::not_null<std::shared_ptr<Material>>& material,
                                                           Axis pole);

    gsl::not_null<std::shared_ptr<Mesh>> m_mesh;
};
}
}
