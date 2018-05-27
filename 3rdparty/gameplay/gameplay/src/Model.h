#pragma once

#include "Drawable.h"
#include "Mesh.h"

#include <gsl/gsl>

namespace gameplay
{
class Model : public Drawable
{
public:
    explicit Model() = default;

    ~Model() = default;

    const std::vector<gsl::not_null<std::shared_ptr<Mesh>>>& getMeshes() const
    {
        return m_meshes;
    }

    void addMesh(const gsl::not_null<std::shared_ptr<Mesh>>& mesh)
    {
        m_meshes.emplace_back( mesh );
    }

    void draw(RenderContext& context) override;

private:

    Model& operator=(const Model&) = delete;

    std::vector<gsl::not_null<std::shared_ptr<Mesh>>> m_meshes{};
};
}
