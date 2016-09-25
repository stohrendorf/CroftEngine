#include "Base.h"
#include "Joint.h"
#include "MeshSkin.h"
#include "Model.h"

#include <glm/gtc/matrix_access.hpp>

namespace gameplay
{
    Joint::Joint(const std::string& id)
        : Node(id)
    {
    }


    Joint::~Joint() = default;


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


    void Joint::updateJointMatrix(glm::vec4* matrixPalette)
    {
        // Note: If more than one MeshSkin influences this Joint, we need to skip
        // the _jointMatrixDirty optimization since updateJointMatrix() may be
        // called multiple times a frame with different bindShape matrices (and
        // different matrixPallete pointers).
        if( _skins.size() > 1 || _jointMatrixDirty )
        {
            _jointMatrixDirty = false;

            const glm::mat4 &t = Node::getWorldMatrix();

            BOOST_ASSERT(matrixPalette);
            matrixPalette[0] = glm::row(t, 0);
            matrixPalette[1] = glm::row(t, 1);
            matrixPalette[2] = glm::row(t, 2);
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
