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
        explicit Model(const std::shared_ptr<Mesh>& mesh = nullptr);
        ~Model();

        /**
         * Returns the Mesh for this Model.
         *
         * @return The Mesh for this Model.
         */
        const std::shared_ptr<Mesh>& getMesh() const;

        /**
         * Sets a material to be used for drawing this Model.
         *
         * The specified Material is applied for the MeshPart at the given index in
         * this Model's Mesh. A partIndex of -1 sets a shared Material for
         * all mesh parts, whereas a value of 0 or greater sets the Material for the
         * specified mesh part only.
         *
         * Mesh parts will use an explicitly set part material, if set; otherwise they
         * will use the globally set material.
         *
         * @param material The new material.
         * @param partIndex The index of the mesh part to set the material for (-1 for shared material).
         */
        void setMaterial(const std::shared_ptr<Material>& material, size_t partIndex);

        /**
         * Sets a material to be used for drawing this Model.
         *
         * A Material is created from the given vertex and fragment shader source files.
         * The Material is applied for the MeshPart at the given index in this Model's
         * Mesh. A partIndex of -1 sets a shared Material for all mesh parts, whereas a
         * value of 0 or greater sets the Material for the specified mesh part only.
         *
         * Mesh parts will use an explicitly set part material, if set; otherwise they
         * will use the globally set material.
         *
         * @param vshPath The path to the vertex shader file.
         * @param fshPath The path to the fragment shader file.
         * @param defines A new-line delimited list of preprocessor defines. May be nullptr.
         * @param partIndex The index of the mesh part to set the material for (-1 for shared material).
         *
         * @return The newly created and bound Material, or nullptr if the Material could not be created.
         */
        std::shared_ptr<Material> setMaterial(const std::string& vshPath, const std::string& fshPath, const std::vector<std::string>& defines = {}, size_t partIndex = -1);

        /**
         * @see Drawable::draw
         *
         * Binds the vertex buffer and index buffers for the Mesh and
         * all of its MeshPart's and draws the mesh geometry.
         * Any other state necessary to render the Mesh, such as
         * rendering states, shader state, and so on, should be set
         * up before calling this method.
         */
        size_t draw(bool wireframe = false) override;


    private:

        Model& operator=(const Model&) = delete;

        /**
         * @see Drawable::setNode
         */
        void setNode(Node* node) override;

        /**
         * Sets the specified material's node binding to this model's node.
         */
        void setMaterialNodeBinding(const std::shared_ptr<Material>& m);

        std::shared_ptr<Mesh> _mesh;
    };
}
