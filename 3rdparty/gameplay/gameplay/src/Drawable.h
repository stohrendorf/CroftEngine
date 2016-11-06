#pragma once

namespace gameplay
{
    class Node;


    /**
     * Defines a drawable object that can be attached to a Node.
     */
    class Drawable : public std::enable_shared_from_this<Drawable>
    {
        friend class Node;

    public:

        /**
         * Constructor.
         */
        Drawable();

        /**
         * Destructor.
         */
        virtual ~Drawable();

        /**
         * Draws the object.
         *
         * @param wireframe true if you want to request to draw the wireframe only.
         * @return The number of graphics draw calls required to draw the object.
         */

        virtual size_t draw(bool wireframe = false) = 0;

        /**
         * Gets the node this drawable is attached to.
         *
         * @return The node this drawable is attached to.
         */
        Node* getNode() const;

    protected:

        /**
         * Sets the node this drawable is attached to.
         *
         * @param node The node this drawable is attached to.
         */
        virtual void setNode(Node* node);

        /**
         * Node this drawable is attached to.
         */
        Node* _node = nullptr;
    };
}
