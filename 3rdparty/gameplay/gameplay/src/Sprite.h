#pragma once

#include "Drawable.h"
#include "Mesh.h"

#include <gsl/gsl>

namespace gameplay
{
class Material;


class Sprite : public Drawable
{
public:
    enum class Axis
    {
        X, Y, Z
    };

    explicit Sprite(const float left,
                    const float bottom,
                    const float width,
                    const float height,
                    const glm::vec2& t0,
                    const glm::vec2& t1,
                    const gsl::not_null<std::shared_ptr<Material>>& material,
                    const Axis pole)
            : m_mesh{createMesh( left, bottom, width, height, t0, t1, material, pole )}
    {
    }

    Sprite(const Sprite&) = delete;

    Sprite(Sprite&&) = delete;

    Sprite& operator=(Sprite&&) = delete;

    Sprite& operator=(const Sprite&) = delete;

    ~Sprite() override = default;

    void draw(RenderContext& context) override;

    RenderState& getRenderState() override
    {
        return m_renderState;
    }

private:
    static gsl::not_null<std::shared_ptr<Mesh>> createMesh(float left,
                                                           float bottom,
                                                           float width,
                                                           float height,
                                                           const glm::vec2& uvTopLeft,
                                                           const glm::vec2& uvBottomRight,
                                                           const gsl::not_null<std::shared_ptr<Material>>& material,
                                                           Axis pole);

    RenderState m_renderState;

    gsl::not_null<std::shared_ptr<Mesh>> m_mesh;
};
}
