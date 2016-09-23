#include "Base.h"
#include "Joint.h"
#include "MeshSkin.h"
#include "Model.h"


namespace gameplay
{
    Joint::Joint(const std::string& id)
        : Node(id)
    {
    }


    Joint::~Joint() = default;


    Node::Type Joint::getType() const
    {
        return Node::JOINT;
    }


    Scene* Joint::getScene() const
    {
        // Overrides Node::getScene() to search the node our skins.
        for( const MeshSkin* itr : _skins )
        {
            std::shared_ptr<Model> model = itr ? itr->getModel() : nullptr;
            if( !model )
                continue;

            Node* node = model->getNode();
            if( !node )
                continue;

            Scene* scene = node->getScene();
            if( scene )
                return scene;
        }

        return Node::getScene();
    }


    void Joint::transformChanged()
    {
        Node::transformChanged();
        _jointMatrixDirty = true;
    }


    void Joint::updateJointMatrix(Vector4* matrixPalette)
    {
        // Note: If more than one MeshSkin influences this Joint, we need to skip
        // the _jointMatrixDirty optimization since updateJointMatrix() may be
        // called multiple times a frame with different bindShape matrices (and
        // different matrixPallete pointers).
        if( _skins.size() > 1 || _jointMatrixDirty )
        {
            _jointMatrixDirty = false;

            const Matrix &t = Node::getWorldMatrix();

            BOOST_ASSERT(matrixPalette);
            matrixPalette[0].set(t.m[0], t.m[4], t.m[8], t.m[12]);
            matrixPalette[1].set(t.m[1], t.m[5], t.m[9], t.m[13]);
            matrixPalette[2].set(t.m[2], t.m[6], t.m[10], t.m[14]);
        }
    }


    void Joint::addSkin(MeshSkin* skin)
    {
        _skins.emplace_back(skin);
    }


    void Joint::removeSkin(MeshSkin* skin)
    {
        _skins.remove(skin);
    }
}
