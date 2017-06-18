#pragma once

#include "Model.h"
#include "Visitor.h"
#include "MaterialParameter.h"

#include <glm/gtc/matrix_transform.hpp>


namespace gameplay
{
    class Drawable;

    class Scene;


    class Node : public std::enable_shared_from_this<Node>
    {
        friend class Scene;

    public:
        using List = std::vector<std::shared_ptr<Node>>;

        explicit Node(const std::string& id);

        virtual ~Node();

        const std::string& getId() const;

        void setId(const std::string& id);

        void addChild(const std::shared_ptr<Node>& child);

        const std::weak_ptr<Node>& getParent() const;

        size_t getChildCount() const;

        Node* getRootNode() const;

        virtual Scene* getScene() const;

        void setVisible(bool enabled);

        bool isVisible() const;

        bool isVisibleInHierarchy() const;

        virtual const glm::mat4& getModelMatrix() const;

        glm::mat4 getModelViewMatrix() const;

        glm::mat4 getInverseTransposeWorldMatrix() const;

        glm::mat4 getInverseTransposeWorldViewMatrix() const;

        const glm::mat4& getViewMatrix() const;

        const glm::mat4& getInverseViewMatrix() const;

        const glm::mat4& getProjectionMatrix() const;

        const glm::mat4& getViewProjectionMatrix() const;

        const glm::mat4& getInverseViewProjectionMatrix() const;

        glm::vec3 getTranslationWorld() const;

        glm::vec3 getTranslationView() const;

        glm::vec3 getActiveCameraTranslationWorld() const;

        const std::shared_ptr<Drawable>& getDrawable() const;

        void setDrawable(const std::shared_ptr<Drawable>& drawable);


        const List& getChildren() const
        {
            return _children;
        }


        const std::shared_ptr<Node>& getChild(size_t idx) const
        {
            BOOST_ASSERT(idx < _children.size());
            return _children[idx];
        }


        const glm::mat4& getLocalMatrix() const
        {
            return m_localMatrix;
        }


        void setLocalMatrix(const glm::mat4& m)
        {
            m_localMatrix = m;
            transformChanged();
        }


        void accept(Visitor& visitor)
        {
            for( auto& node : _children )
                visitor.visit(*node);
        }


        void setParent(const std::shared_ptr<Node>& parent)
        {
            if( !_parent.expired() )
            {
                auto p = _parent.lock();
                auto it = find(p->_children.begin(), p->_children.end(), shared_from_this());
                BOOST_ASSERT(it != p->_children.end());
                _parent.lock()->_children.erase(it);
            }

            _parent = parent;

            if( parent != nullptr )
                parent->_children.push_back(shared_from_this());

            transformChanged();
        }


        void swapChildren(const std::shared_ptr<Node>& other)
        {
            auto otherChildren = other->_children;
            for( auto& child : otherChildren )
                child->setParent(nullptr);
            BOOST_ASSERT(other->_children.empty());

            auto thisChildren = _children;
            for( auto& child : thisChildren )
                child->setParent(nullptr);
            BOOST_ASSERT(_children.empty());

            for( auto& child : otherChildren )
                child->setParent(shared_from_this());

            for( auto& child : thisChildren )
                child->setParent(other);
        }


        void addMaterialParameterSetter(const std::string& name, const std::function<MaterialParameter::UniformValueSetter>& setter)
        {
            _materialParemeterSetters[name] = setter;
        }


        void addMaterialParameterSetter(const std::string& name, std::function<MaterialParameter::UniformValueSetter>&& setter)
        {
            _materialParemeterSetters[name] = move(setter);
        }


        const std::map<std::string, std::function<MaterialParameter::UniformValueSetter>>& getMaterialParameterSetters() const
        {
            return _materialParemeterSetters;
        }


    protected:

        void transformChanged();

    private:

        Node(const Node& copy) = delete;

        Node& operator=(const Node&) = delete;

        Scene* _scene = nullptr;

        std::string _id;

        List _children;

        std::weak_ptr<Node> _parent{};

        bool m_visible = true;

        std::shared_ptr<Drawable> _drawable = nullptr;

        glm::mat4 m_localMatrix{1.0f};

        mutable glm::mat4 m_modelMatrix{1.0f};

        mutable bool _dirty = false;

        std::map<std::string, std::function<MaterialParameter::UniformValueSetter>> _materialParemeterSetters;
    };
}
