#include "Base.h"
#include "MeshSkin.h"
#include "Joint.h"
#include "Model.h"

// The number of rows in each palette matrix.
#define PALETTE_ROWS 3


namespace gameplay
{
    MeshSkin::MeshSkin()
        : _rootJoint(NULL)
        , _rootNode(NULL)
        , _matrixPalette(NULL)
        , _model(NULL)
    {
    }


    MeshSkin::~MeshSkin()
    {
        clearJoints();

        SAFE_DELETE_ARRAY(_matrixPalette);
    }


    const Matrix& MeshSkin::getBindShape() const
    {
        return _bindShape;
    }


    void MeshSkin::setBindShape(const float* matrix)
    {
        _bindShape.set(matrix);
    }


    unsigned int MeshSkin::getJointCount() const
    {
        return (unsigned int)_joints.size();
    }


    Joint* MeshSkin::getJoint(unsigned int index) const
    {
        GP_ASSERT(index < _joints.size());
        return _joints[index];
    }


    Joint* MeshSkin::getJoint(const char* id) const
    {
        GP_ASSERT(id);

        for( size_t i = 0, count = _joints.size(); i < count; ++i )
        {
            Joint* j = _joints[i];
            if( j && j->getId() != NULL && strcmp(j->getId(), id) == 0 )
            {
                return j;
            }
        }

        return NULL;
    }


    void MeshSkin::setJointCount(unsigned int jointCount)
    {
        // Erase the joints vector and release all joints.
        clearJoints();

        // Resize the joints vector and initialize to NULL.
        _joints.resize(jointCount);
        for( unsigned int i = 0; i < jointCount; i++ )
        {
            _joints[i] = NULL;
        }

        // Rebuild the matrix palette. Each matrix is 3 rows of Vector4.
        SAFE_DELETE_ARRAY(_matrixPalette);

        if( jointCount > 0 )
        {
            _matrixPalette = new Vector4[jointCount * PALETTE_ROWS];
            for( unsigned int i = 0; i < jointCount * PALETTE_ROWS; i += PALETTE_ROWS )
            {
                _matrixPalette[i + 0].set(1.0f, 0.0f, 0.0f, 0.0f);
                _matrixPalette[i + 1].set(0.0f, 1.0f, 0.0f, 0.0f);
                _matrixPalette[i + 2].set(0.0f, 0.0f, 1.0f, 0.0f);
            }
        }
    }


    void MeshSkin::setJoint(Joint* joint, unsigned int index)
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
        GP_ASSERT(_matrixPalette);

        for( size_t i = 0, count = _joints.size(); i < count; i++ )
        {
            GP_ASSERT(_joints[i]);
            _joints[i]->updateJointMatrix(getBindShape(), &_matrixPalette[i * PALETTE_ROWS]);
        }
        return _matrixPalette;
    }


    unsigned int MeshSkin::getMatrixPaletteSize() const
    {
        return (unsigned int)_joints.size() * PALETTE_ROWS;
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
            for( Node* node = newRootNode->getParent(); node != NULL; node = node->getParent() )
            {
                if( node->getParent() == NULL )
                {
                    newRootNode = node;
                    break;
                }
            }
        }
        setRootNode(newRootNode);
    }


    void MeshSkin::transformChanged(Transform* transform, long cookie)
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
        setRootJoint(NULL);

        for( size_t i = 0, count = _joints.size(); i < count; ++i )
        {
            SAFE_RELEASE(_joints[i]);
        }
        _joints.clear();
    }
}
