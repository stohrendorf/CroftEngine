#include "Base.h"
#include "Technique.h"
#include "Material.h"
#include "Node.h"


namespace gameplay
{
    Technique::Technique(const char* id, const std::shared_ptr<Material>& material)
        : _id(id ? id : "")
        , _material(material)
    {
        RenderState::_parent = material;
    }


    Technique::~Technique() = default;


    const std::string& Technique::getId() const
    {
        return _id;
    }


    std::shared_ptr<Pass> Technique::getPass() const
    {
        return _pass;
    }


    void Technique::setNodeBinding(Node* node)
    {
        RenderState::setNodeBinding(node);
        _pass->setNodeBinding(node);
    }
}
