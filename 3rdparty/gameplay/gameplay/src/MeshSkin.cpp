#include "Base.h"
#include "MeshSkin.h"
#include "Joint.h"
#include "Model.h"

// The number of rows in each palette matrix.
#define PALETTE_ROWS 3


namespace gameplay
{
    MeshSkin::MeshSkin()
        : _rootJoint(nullptr)
        , _rootNode(nullptr)
        , _matrixPalette()
        , _model(nullptr)
    {
    }


    MeshSkin::~MeshSkin()
    {
        clearJoints();
    }


    size_t MeshSkin::getJointCount() const
    {
        return _joints.size();
    }


    Joint* MeshSkin::getJoint(size_t index) const
    {
        GP_ASSERT(index < _joints.size());
        return _joints[index];
    }


    Joint* MeshSkin::getJoint(const std::string& id) const
    {
        for( size_t i = 0, count = _joints.size(); i < count; ++i )
        {
            Joint* j = _joints[i];
            if( j && j->getId() == id )
            {
                return j;
            }
        }

        return nullptr;
    }


    void MeshSkin::setJointCount(size_t jointCount)
    {
        // Erase the joints vector and release all joints.
        clearJoints();

        // Resize the joints vector and initialize to nullptr.
        _joints.resize(jointCount);
        for( size_t i = 0; i < jointCount; i++ )
        {
            _joints[i] = nullptr;
        }

        // Rebuild the matrix palette. Each matrix is 3 rows of Vector4.
        _matrixPalette.clear();
        if( jointCount > 0 )
        {
            _matrixPalette.resize(jointCount * PALETTE_ROWS);
            for(size_t i = 0; i < jointCount * PALETTE_ROWS; i += PALETTE_ROWS )
            {
                _matrixPalette[i + 0].set(1.0f, 0.0f, 0.0f, 0.0f);
                _matrixPalette[i + 1].set(0.0f, 1.0f, 0.0f, 0.0f);
                _matrixPalette[i + 2].set(0.0f, 0.0f, 1.0f, 0.0f);
            }
        }
    }


    void MeshSkin::setJoint(Joint* joint, size_t index)
    {
        GP_ASSERT(index < _joints.size());

        if( _joints[index] )
        {
            _joints[index]->removeSkin(this);
            SAFE_RELEASE(_joints[index]);
        }

        _joints[index] = joint;

        if( joint )
        {
            joint->addRef();
            joint->addSkin(this);
        }
    }


    Vector4* MeshSkin::getMatrixPalette() const
    {
        for( size_t i = 0, count = _joints.size(); i < count; i++ )
        {
            GP_ASSERT(_joints[i]);
            _joints[i]->updateJointMatrix(&_matrixPalette[i * PALETTE_ROWS]);
        }
        return _matrixPalette.data();
    }


    size_t MeshSkin::getMatrixPaletteSize() const
    {
        return _joints.size() * PALETTE_ROWS;
    }


    Model* MeshSkin::getModel() const
    {
        return _model;
    }


    Joint* MeshSkin::getRootJoint() const
    {
        return _rootJoint;
    }


    void MeshSkin::setRootJoint(Joint* joint)
    {
        if( _rootJoint )
        {
            if( _rootJoint->getParent() )
            {
                _rootJoint->getParent()->removeListener(this);
            }
        }

        _rootJoint = joint;

        // If the root joint has a parent node, register for its transformChanged event
        if( _rootJoint && _rootJoint->getParent() )
        {
            _rootJoint->getParent()->addListener(this, 1);
        }

        Node* newRootNode = _rootJoint;
        if( newRootNode )
        {
            // Find the top level parent node of the root joint
            for( Node* node = newRootNode->getParent(); node != nullptr; node = node->getParent() )
            {
                if( node->getParent() == nullptr )
                {
                    newRootNode = node;
                    break;
                }
            }
        }
        setRootNode(newRootNode);
    }


    void MeshSkin::transformChanged(Transform* /*transform*/, long cookie)
    {
        switch( cookie )
        {
            case 1:
                // The direct parent of our joint hierarchy has changed.
                // Dirty the bounding volume for our model's node. This special
                // case allows us to have much tighter bounding volumes for
                // skinned meshes by only considering local skin/joint transformations
                // during bounding volume computation instead of fully resolved
                // joint transformations.
                if( _model && _model->getNode() )
                {
                    _model->getNode()->setBoundsDirty();
                }
                break;
        }
    }


    int MeshSkin::getJointIndex(Joint* joint) const
    {
        for( size_t i = 0, count = _joints.size(); i < count; ++i )
        {
            if( _joints[i] == joint )
            {
                return (int)i;
            }
        }

        return -1;
    }


    void MeshSkin::setRootNode(Node* node)
    {
        if( _rootNode != node )
        {
            SAFE_RELEASE(_rootNode);
            _rootNode = node;
            if( _rootNode )
            {
                _rootNode->addRef();
            }
        }
    }


    void MeshSkin::clearJoints()
    {
        setRootJoint(nullptr);

        for( size_t i = 0, count = _joints.size(); i < count; ++i )
        {
            SAFE_RELEASE(_joints[i]);
        }
        _joints.clear();
    }
}
