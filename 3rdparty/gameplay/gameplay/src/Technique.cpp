#include "Base.h"
#include "Technique.h"
#include "Material.h"
#include "Node.h"

namespace gameplay
{

Technique::Technique(const char* id, const std::shared_ptr<Material>& material)
    : _id(id ? id : ""), _material(material)
{
    RenderState::_parent = material;
}

Technique::~Technique() = default;

const char* Technique::getId() const
{
    return _id.c_str();
}

size_t Technique::getPassCount() const
{
    return _passes.size();
}

const std::shared_ptr<Pass>& Technique::getPassByIndex(size_t index) const
{
    GP_ASSERT(index < _passes.size());
    return _passes[index];
}


    std::shared_ptr<Pass> Technique::getPass(const char* id) const
{
    GP_ASSERT(id);

    for (size_t i = 0, count = _passes.size(); i < count; ++i)
    {
        auto pass = _passes[i];
        GP_ASSERT(pass);
        if (strcmp(pass->getId(), id) == 0)
        {
            return pass;
        }
    }
    return nullptr;
}

void Technique::setNodeBinding(Node* node)
{
    RenderState::setNodeBinding(node);

    for (size_t i = 0, count = _passes.size(); i < count; ++i)
    {
        _passes[i]->setNodeBinding(node);
    }
}

}
