#pragma once

#include "Node.h"


namespace gameplay
{
    class MeshSkin;


    /**
     * Defines a joint node.
     *
     * This represent a joint in a skeleton that is hierarchially part of
     * a MeshSkin. This allows the vertices in the mesh to be blended and
     * animated using the sum of the blend weight that must add up to 1.0.
     */
    class Joint : public Node
    {
        friend class Node;
        friend class MeshSkin;

    public:

        explicit Joint(const std::string& id);
        virtual ~Joint();

        /**
         * @see Node::getType()
         */
        Node::Type getType() const override;

        /**
         * @see Node::getScene()
         */
        Scene* getScene() const override;

    protected:

        /**
         * Updates the joint matrix.
         *
         * @param matrixPalette The matrix palette to update.
         */
        void updateJointMatrix(Vector4* matrixPalette);

        /**
         * Called when this Joint's transform changes.
         */
        void transformChanged() override;

    private:

        Joint(const Joint& copy) = delete;
        Joint& operator=(const Joint&) = delete;

        void addSkin(MeshSkin* skin);
        void removeSkin(MeshSkin* skin);

        /**
         * Flag used to mark if the Joint's matrix is dirty.
         */
        bool _jointMatrixDirty = true;

        /**
         * Linked list of mesh skins that are referenced by this joint.
         */
        std::list<MeshSkin*> _skins;
    };
}
