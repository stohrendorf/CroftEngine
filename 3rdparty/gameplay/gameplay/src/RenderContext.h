#pragma once

namespace gameplay
{
class Node;


class RenderContext
{
public:
    explicit RenderContext() = default;

    Node* getCurrentNode() const noexcept
    {
        return m_currentNode;
    }

    void setCurrentNode(Node* n) noexcept
    {
        m_currentNode = n;
    }

private:
    Node* m_currentNode = nullptr;
};
}
