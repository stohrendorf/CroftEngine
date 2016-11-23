#pragma once

#include "Base.h"

#include <memory>

namespace gameplay
{
    class Node;


    class RenderContext
    {
    public:
        explicit RenderContext(bool wireframe = false)
                : m_wireframe{wireframe}
        {}


        Node* getCurrentNode() const noexcept
        {
            return m_currentNode;
        }


        void setCurrentNode(Node* n) noexcept
        {
            m_currentNode = n;
        }


        bool isWireframe() const noexcept
        {
            return m_wireframe;
        }


    private:
        Node* m_currentNode = nullptr;
        const bool m_wireframe;
    };


    class Drawable : public std::enable_shared_from_this<Drawable>
    {
    public:
        explicit Drawable();

        virtual ~Drawable();

        virtual void draw(RenderContext& context) = 0;
    };
}
