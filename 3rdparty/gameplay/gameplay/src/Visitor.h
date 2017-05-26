#pragma once

namespace gameplay
{
    class RenderContext;

    class Node;


    class Visitor
    {
    public:
        explicit Visitor(RenderContext& context)
            : m_context{context}
        {
        }


        virtual ~Visitor() = default;

        virtual void visit(Node& node);


        RenderContext& getContext()
        {
            return m_context;
        }


    private:
        RenderContext& m_context;
    };
}
