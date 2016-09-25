#pragma once

#include "Transform.h"


namespace gameplay
{
    class Model;
    class Node;
    class Joint;


    /**
     * Defines the skin for a mesh.
     *
     * A skin is used to support skinning otherwise known as
     * vertex blending. This allows for a Model's mesh to support
     * a skeleton on joints that will influence the vertex position
     * and which the joints can be animated.
     */
    class MeshSkin : public Transform::Listener
    {
        friend class Model;
        friend class Joint;
        friend class Node;
        friend class Scene;

    public:

        MeshSkin();
        ~MeshSkin();

        /**
         * Returns the number of joints in this MeshSkin.
         */
        size_t getJointCount() const;

        /**
         * Returns the joint at the given index.
         *
         * @param index The index.
         *
         * @return The joint.
         */
        const std::shared_ptr<Joint>& getJoint(size_t index) const;

        /**
         * Returns the joint with the given ID.
         *
         * @param id The ID of the joint to search for.
         *
         * @return The joint, or NULL if not found.
         */
        std::shared_ptr<Joint> getJoint(const std::string& id) const;

        /**
         * Returns the root most joint for this MeshSkin.
         *
         * @return The root joint.
         */
        const std::shared_ptr<Joint>& getRootJoint() const;

        /**
         * Sets the root joint for this MeshSkin.
         *
         * The specified Joint must belong to the joint list for this MeshSkin.
         *
         * @param joint The root joint.
         */
        void setRootJoint(const std::shared_ptr<Joint>& joint);

        /**
         * Returns the index of the specified joint in this MeshSkin.
         *
         * @return The index of the joint in this MeshSkin, or -1 if the joint does not belong to this MeshSkin.
         */
        int getJointIndex(const std::shared_ptr<Joint>& joint) const;

        /**
         * Returns the pointer to the glm::vec4 array for the purpose of binding to a shader.
         *
         * @return The pointer to the matrix palette.
         */
        glm::vec4* getMatrixPalette() const;

        /**
         * Returns the number of elements in the matrix palette array.
         * Each element is a glm::vec4* that represents a row.
         * Each matrix palette is represented by 3 rows of glm::vec4.
         *
         * @return The matrix palette size.
         */
        size_t getMatrixPaletteSize() const;

        /**
         * Returns our parent Model.
         */
        const std::shared_ptr<Model>& getModel() const;

        /**
         * Handles transform change events for joints.
         */
        void transformChanged(Transform* transform, long cookie) override;

        /**
        * Sets the number of joints that can be stored in this skin.
        * This method allocates the necessary memory.
        *
        * @param jointCount The new size of the joint vector.
        */
        void setJointCount(size_t jointCount);

        /**
        * Sets the joint at the given index and increments the ref count.
        *
        * @param joint The joint to be set.
        * @param index The index in the joints vector.
        */
        void setJoint(const std::shared_ptr<Joint>& joint, size_t index);

    private:

        MeshSkin(const MeshSkin&) = delete;
        MeshSkin& operator=(const MeshSkin&) = delete;

        /**
         * Sets the root node of this mesh skin.
         *
         * @param node The node to set as the root node, may be NULL.
         */
        void setRootNode(const std::shared_ptr<Node>& node);

        /**
         * Clears the list of joints and releases each joint.
         */
        void clearJoints();

        std::vector<std::shared_ptr<Joint>> _joints;
        std::shared_ptr<Joint> _rootJoint;

        // Pointer to the root node of the mesh skin.
        // The purpose is so that the joint hierarchy doesn't need to be in the scene.
        // If the joints are not in the scene then something has to hold a reference to it.
        std::shared_ptr<Node> _rootNode;

        // Pointer to the array of palette matrices.
        // This array is passed to the vertex shader as a uniform.
        // Each 4x3 row-wise matrix is represented as 3 glm::vec4's.
        // The number of glm::vec4's is (_joints.size() * 3).
        mutable std::vector<glm::vec4> _matrixPalette;
        std::shared_ptr<Model> _model;
    };
}
