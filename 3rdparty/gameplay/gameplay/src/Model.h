#pragma once

#include "Drawable.h"
#include "Mesh.h"


namespace gameplay
{
    class Model : public Drawable
    {
    public:
        explicit Model() = default;

        ~Model() = default;


        const std::vector<std::shared_ptr<Mesh>>& getMeshes() const
        {
            return m_meshes;
        }


        void addMesh(const std::shared_ptr<Mesh>& mesh)
        {
            BOOST_ASSERT(mesh != nullptr);
            m_meshes.push_back(mesh);
        }


        void draw(RenderContext& context) override;


    private:

        Model& operator=(const Model&) = delete;

        std::vector<std::shared_ptr<Mesh>> m_meshes{};
    };
}
