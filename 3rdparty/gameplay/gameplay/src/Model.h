#pragma once

#include "Mesh.h"
#include "Material.h"
#include "Drawable.h"


namespace gameplay
{
    /**
     * Defines a Model or mesh renderer which is an instance of a Mesh.
     *
     * A model has a mesh that can be drawn with the specified materials for
     * each of the mesh parts within it.
     */
    class Model : public Drawable
    {
        friend class Node;
        friend class Scene;
        friend class Mesh;

    public:
        explicit Model();
        ~Model();

        /**
         * Returns the Mesh for this Model.
         *
         * @return The Mesh for this Model.
         */
        const std::vector<std::shared_ptr<Mesh>>& getMeshes() const
        {
            return _meshes;
        }


        void addMesh(const std::shared_ptr<Mesh>& mesh)
        {
            BOOST_ASSERT(mesh != nullptr);
            _meshes.push_back(mesh);
        }

        /**
         * @see Drawable::draw
         *
         * Binds the vertex buffer and index buffers for the Mesh and
         * all of its MeshPart's and draws the mesh geometry.
         * Any other state necessary to render the Mesh, such as
         * rendering states, shader state, and so on, should be set
         * up before calling this method.
         */
        void draw(RenderContext& context) override;


    private:

        Model& operator=(const Model&) = delete;

        std::vector<std::shared_ptr<Mesh>> _meshes;
    };
}
