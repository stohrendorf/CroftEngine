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
    Node(const Node& copy) = delete;

    Node& operator=(const Node&) = delete;

    using List = std::vector<gsl::not_null<std::shared_ptr<Node>>>;

    explicit Node(const std::string& id);

    virtual ~Node();

    const std::string& getId() const;

    void addChild(const std::shared_ptr<Node>& child);

    const std::weak_ptr<Node>& getParent() const;

    size_t getChildCount() const;

    virtual Scene* getScene() const;

    void setVisible(bool enabled);

    bool isVisible() const;

    bool isVisibleInHierarchy() const;

    virtual const glm::mat4& getModelMatrix() const;

    glm::mat4 getModelViewMatrix() const;

    const glm::mat4& getViewMatrix() const;

    const glm::mat4& getInverseViewMatrix() const;

    const glm::mat4& getProjectionMatrix() const;

    const glm::mat4& getViewProjectionMatrix() const;

    const glm::mat4& getInverseViewProjectionMatrix() const;

    glm::vec3 getTranslationWorld() const;

    const std::shared_ptr<Drawable>& getDrawable() const;

    void setDrawable(const std::shared_ptr<Drawable>& drawable);

    const List& getChildren() const
    {
        return m_children;
    }

    const gsl::not_null<std::shared_ptr<Node>>& getChild(size_t idx) const
    {
        BOOST_ASSERT( idx < m_children.size() );
        return m_children[idx];
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
        for( auto& node : m_children )
            visitor.visit( *node );
    }

    void setParent(const std::shared_ptr<Node>& parent)
    {
        if( !m_parent.expired() )
        {
            auto p = m_parent.lock();
            gsl::not_null<std::shared_ptr<Node>> self = shared_from_this();
            auto it = std::find( p->m_children.begin(), p->m_children.end(), self );
            BOOST_ASSERT( it != p->m_children.end() );
            m_parent.lock()->m_children.erase( it );
        }

        m_parent = parent;

        if( parent != nullptr )
            parent->m_children.emplace_back( shared_from_this() );

        transformChanged();
    }

    void swapChildren(const gsl::not_null<std::shared_ptr<Node>>& other)
    {
        auto otherChildren = other->m_children;
        for( auto& child : otherChildren )
            child->setParent( nullptr );
        BOOST_ASSERT( other->m_children.empty() );

        auto thisChildren = m_children;
        for( auto& child : thisChildren )
            child->setParent( nullptr );
        BOOST_ASSERT( m_children.empty() );

        for( auto& child : otherChildren )
            child->setParent( shared_from_this() );

        for( auto& child : thisChildren )
            child->setParent( other );
    }

    void addMaterialParameterSetter(const std::string& name,
                                    const std::function<MaterialParameter::UniformValueSetter>& setter)
    {
        m_materialParameterSetters[name] = setter;
    }

    void addMaterialParameterSetter(const std::string& name,
                                    std::function<MaterialParameter::UniformValueSetter>&& setter)
    {
        m_materialParameterSetters[name] = std::move( setter );
    }

    const std::function<MaterialParameter::UniformValueSetter>*
    findMaterialParameterSetter(const std::string& name) const
    {
        auto it = m_materialParameterSetters.find( name );
        if( it != m_materialParameterSetters.end() )
            return &it->second;

        if( !m_parent.expired() )
            return m_parent.lock()->findMaterialParameterSetter( name );

        return nullptr;
    }

protected:
    void transformChanged();

private:
    Scene* m_scene = nullptr;

    std::string m_id;

    List m_children;

    std::weak_ptr<Node> m_parent{};

    bool m_visible = true;

    std::shared_ptr<Drawable> m_drawable = nullptr;

    glm::mat4 m_localMatrix{1.0f};

    mutable glm::mat4 m_modelMatrix{1.0f};

    mutable bool m_dirty = false;

    std::map<std::string, std::function<MaterialParameter::UniformValueSetter>> m_materialParameterSetters;
};
}
