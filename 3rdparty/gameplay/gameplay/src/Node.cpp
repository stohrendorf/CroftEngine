#include "Base.h"
#include "Node.h"
#include "Scene.h"
#include "Camera.h"

namespace gameplay
{
    Node::Node(const std::string& id)
            : m_id( id )
    {
    }

    Node::~Node()
    {
        setParent( nullptr );
    }

    const std::string& Node::getId() const
    {
        return m_id;
    }

    void Node::addChild(const std::shared_ptr<Node>& child)
    {
        BOOST_ASSERT( child );

        if( !child->m_parent.expired() && child->m_parent.lock() == shared_from_this() )
        {
            // This node is already present in our hierarchy
            return;
        }

        child->setParent( shared_from_this() );
    }

    const std::weak_ptr<Node>& Node::getParent() const
    {
        return m_parent;
    }

    size_t Node::getChildCount() const
    {
        return m_children.size();
    }

    Scene* Node::getScene() const
    {
        if( m_scene )
            return m_scene;

        // Search our parent for the scene
        if( !m_parent.expired() )
        {
            Scene* scene = m_parent.lock()->getScene();
            if( scene )
                return scene;
        }
        return nullptr;
    }

    void Node::setVisible(bool visible)
    {
        m_visible = visible;
    }

    bool Node::isVisible() const
    {
        return m_visible;
    }

    bool Node::isVisibleInHierarchy() const
    {
        if( !m_visible )
            return false;

        auto node = m_parent;
        while( !node.expired() )
        {
            if( !node.lock()->m_visible )
            {
                return false;
            }
            node = node.lock()->m_parent;
        }
        return true;
    }

    const glm::mat4& Node::getModelMatrix() const
    {
        if( m_dirty )
        {
            // Clear our dirty flag immediately to prevent this block from being entered if our
            // parent calls our getModelMatrix() method as a result of the following calculations.
            m_dirty = false;

            // If we have a parent, multiply our parent world transform by our local
            // transform to obtain our final resolved world transform.
            auto parent = getParent();
            if( !parent.expired() )
            {
                m_modelMatrix = parent.lock()->getModelMatrix() * getLocalMatrix();
            }
            else
            {
                m_modelMatrix = getLocalMatrix();
            }
        }
        return m_modelMatrix;
    }

    glm::mat4 Node::getModelViewMatrix() const
    {
        return getViewMatrix() * getModelMatrix();
    }

    const glm::mat4& Node::getViewMatrix() const
    {
        Scene* scene = getScene();
        auto camera = scene ? scene->getActiveCamera() : nullptr;
        if( camera )
        {
            return camera->getViewMatrix();
        }
        else
        {
            static const glm::mat4 identity{1.0f};
            return identity;
        }
    }

    const glm::mat4& Node::getInverseViewMatrix() const
    {
        Scene* scene = getScene();
        auto camera = scene ? scene->getActiveCamera() : nullptr;
        if( camera )
        {
            return camera->getInverseViewMatrix();
        }
        else
        {
            static const glm::mat4 identity{1.0f};
            return identity;
        }
    }

    const glm::mat4& Node::getProjectionMatrix() const
    {
        Scene* scene = getScene();
        auto camera = scene ? scene->getActiveCamera() : nullptr;
        if( camera )
        {
            return camera->getProjectionMatrix();
        }
        else
        {
            static const glm::mat4 identity{1.0f};
            return identity;
        }
    }

    const glm::mat4& Node::getViewProjectionMatrix() const
    {
        Scene* scene = getScene();
        auto camera = scene ? scene->getActiveCamera() : nullptr;
        if( camera )
        {
            return camera->getViewProjectionMatrix();
        }
        else
        {
            static const glm::mat4 identity{1.0f};
            return identity;
        }
    }

    const glm::mat4& Node::getInverseViewProjectionMatrix() const
    {
        Scene* scene = getScene();
        auto camera = scene ? scene->getActiveCamera() : nullptr;
        if( camera )
        {
            return camera->getInverseViewProjectionMatrix();
        }
        static const glm::mat4 identity{1.0f};
        return identity;
    }

    glm::vec3 Node::getTranslationWorld() const
    {
        return glm::vec3( getModelMatrix()[3] );
    }

    void Node::transformChanged()
    {
        m_dirty = true;

        // Notify our children that their transform has also changed (since transforms are inherited).
        for( const auto& child : m_children )
        {
            child->transformChanged();
        }
    }

    const std::shared_ptr<Drawable>& Node::getDrawable() const
    {
        return m_drawable;
    }

    void Node::setDrawable(const std::shared_ptr<Drawable>& drawable)
    {
        m_drawable = drawable;
    }
}
