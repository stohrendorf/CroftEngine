#pragma once

#include "Drawable.h"
#include "Mesh.h"


namespace gameplay
{
    class Model : public Drawable
    {
    public:
        explicit Model();

        ~Model();


        const std::vector<std::shared_ptr<Mesh>>& getMeshes() const
        {
            return _meshes;
        }


        void addMesh(const std::shared_ptr<Mesh>& mesh)
        {
            BOOST_ASSERT(mesh != nullptr);
            _meshes.push_back(mesh);
        }


        void draw(RenderContext& context) override;


    private:

        Model& operator=(const Model&) = delete;

        std::vector<std::shared_ptr<Mesh>> _meshes;
    };
}
