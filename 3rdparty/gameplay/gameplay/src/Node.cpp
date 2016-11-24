#include "Base.h"
#include "Node.h"
#include "Scene.h"
#include "Camera.h"

namespace gameplay
{
    Node::Node(const std::string& id)
        : _id(id)
    {
    }


    Node::~Node()
    {
        setParent(nullptr);
    }


    const std::string& Node::getId() const
    {
        return _id;
    }


    void Node::setId(const std::string& id)
    {
        _id = id;
    }


    void Node::addChild(const std::shared_ptr<Node>& child)
    {
        BOOST_ASSERT(child);

        if( child->_parent.lock() == shared_from_this() )
        {
            // This node is already present in our hierarchy
            return;
        }

        _children.push_back(child);
        child->setParent(shared_from_this());
    }


    const std::weak_ptr<Node>& Node::getParent() const
    {
        return _parent;
    }


    size_t Node::getChildCount() const
    {
        return _children.size();
    }


    Node* Node::getRootNode() const
    {
        Node* n = const_cast<Node*>(this);
        while( !n->getParent().expired() )
        {
            n = n->getParent().lock().get();
        }
        return n;
    }


    Scene* Node::getScene() const
    {
        if( _scene )
            return _scene;

        // Search our parent for the scene
        if( !_parent.expired() )
        {
            Scene* scene = _parent.lock()->getScene();
            if( scene )
                return scene;
        }
        return nullptr;
    }


    void Node::setEnabled(bool enabled)
    {
        _enabled = enabled;
    }


    bool Node::isEnabled() const
    {
        return _enabled;
    }


    bool Node::isEnabledInHierarchy() const
    {
        if( !_enabled )
            return false;

        auto node = _parent;
        while( !node.expired() )
        {
            if( !node.lock()->_enabled )
            {
                return false;
            }
            node = node.lock()->_parent;
        }
        return true;
    }


    const glm::mat4& Node::getWorldMatrix() const
    {
        if( _dirty )
        {
            // Clear our dirty flag immediately to prevent this block from being entered if our
            // parent calls our getWorldMatrix() method as a result of the following calculations.
            _dirty = false;

            // If we have a parent, multiply our parent world transform by our local
            // transform to obtain our final resolved world transform.
            auto parent = getParent();
            if( !parent.expired() )
            {
                m_worldMatrix = parent.lock()->getWorldMatrix() * getLocalMatrix();
            }
            else
            {
                m_worldMatrix = getLocalMatrix();
            }
        }
        return m_worldMatrix;
    }


    glm::mat4 Node::getWorldViewMatrix() const
    {
        return getViewMatrix() * getWorldMatrix();
    }


    glm::mat4 Node::getInverseTransposeWorldViewMatrix() const
    {
        return glm::transpose(glm::inverse(getViewMatrix() * getWorldMatrix()));
    }


    glm::mat4 Node::getInverseTransposeWorldMatrix() const
    {
        return glm::transpose(glm::inverse(getWorldMatrix()));
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


    glm::mat4 Node::getWorldViewProjectionMatrix() const
    {
        // Always re-calculate worldViewProjection matrix since it's extremely difficult
        // to track whether the camera has changed (it may frequently change every frame).
        return getViewProjectionMatrix() * getWorldMatrix();
    }


    glm::vec3 Node::getTranslationWorld() const
    {
        return glm::vec3(getWorldMatrix()[3]);
    }


    glm::vec3 Node::getTranslationView() const
    {
        return glm::vec3(glm::vec4(getTranslationWorld(), 1.0f) * getViewMatrix());
    }


    glm::vec3 Node::getActiveCameraTranslationWorld() const
    {
        Scene* scene = getScene();
        if( !scene )
            return{ 0,0,0 };

        if( auto camera = scene->getActiveCamera() )
            return glm::vec3(camera->getInverseViewMatrix()[3]);

        return{ 0,0,0 };
    }


    void Node::transformChanged()
    {
        _dirty = true;

        // Notify our children that their transform has also changed (since transforms are inherited).
        for( const auto& child : _children )
        {
            child->transformChanged();
        }
    }

    const std::shared_ptr<Light>& Node::getLight() const
    {
        return _light;
    }


    void Node::setLight(const std::shared_ptr<Light>& light)
    {
        if( _light == light )
            return;

        if( _light )
        {
            _light->setNode(nullptr);
        }

        _light = light;

        if( _light )
        {
            _light->setNode(this);
        }
    }


    const std::shared_ptr<Drawable>& Node::getDrawable() const
    {
        return _drawable;
    }


    void Node::setDrawable(const std::shared_ptr<Drawable>& drawable)
    {
        _drawable = drawable;
    }
}
