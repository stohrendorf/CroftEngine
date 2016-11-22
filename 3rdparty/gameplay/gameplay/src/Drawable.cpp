#include "Base.h"
#include "Drawable.h"
#include "Node.h"


namespace gameplay
{

Drawable::Drawable() = default;

Drawable::~Drawable() = default;

Node* Drawable::getNode() const
{
    return _node;
}

void Drawable::setNode(Node* node)
{
    _node = node;
}

}
