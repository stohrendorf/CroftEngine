#include "Base.h"
#include "Model.h"
#include "MeshPart.h"
#include "Node.h"
#include "MaterialParameter.h"

#include <boost/log/trivial.hpp>

namespace gameplay
{
    Model::Model(const std::shared_ptr<Mesh>& mesh)
        : Drawable()
        , _mesh(mesh)
    {
        BOOST_ASSERT(mesh);
    }


    Model::~Model() = default;


    const std::shared_ptr<Mesh>& Model::getMesh() const
    {
        return _mesh;
    }


    void Model::setMaterial(const std::shared_ptr<Material>& material, size_t partIndex)
    {
        BOOST_ASSERT(partIndex < _mesh->getPartCount());
        BOOST_ASSERT(material != nullptr);

        _mesh->getPart(partIndex)->setMaterial(material);
    }


    std::shared_ptr<Material> Model::setMaterial(const std::string& vshPath, const std::string& fshPath, const std::vector<std::string>& defines, size_t partIndex)
    {
        // Try to create a Material with the given parameters.
        auto material = std::make_shared<Material>(vshPath, fshPath, defines);
        if( material == nullptr )
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to create material for model.";
            return nullptr;
        }

        // Assign the material to us.
        setMaterial(material, partIndex);

        return material;
    }


    void Model::draw(RenderContext& context)
    {
        BOOST_ASSERT(_mesh);

        for(const auto& part : _mesh->_parts)
        {
            BOOST_ASSERT(part);

            part->draw(context);
        }
    }
}
