#pragma once

#include "gsl-lite.hpp"
#include "renderable.h"

namespace render
{
namespace scene
{
class Mesh;

class Model : public Renderable
{
public:
    explicit Model() = default;

    ~Model() override = default;

    Model(const Model&) = delete;

    Model(Model&&) = delete;

    Model& operator=(Model&&) = delete;

    Model& operator=(const Model&) = delete;

    const auto& getMeshes() const
    {
        return m_meshes;
    }

    auto& getMeshes()
    {
        return m_meshes;
    }

    void addMesh(const gsl::not_null<std::shared_ptr<Mesh>>& mesh)
    {
        m_meshes.emplace_back(mesh);
    }

    void render(RenderContext& context) override;

private:
    std::vector<gsl::not_null<std::shared_ptr<Mesh>>> m_meshes{};
};
} // namespace scene
} // namespace render
