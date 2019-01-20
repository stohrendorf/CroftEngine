#include "Base.h"
#include "Material.h"
#include "ShaderProgram.h"
#include "Node.h"
#include "MaterialParameter.h"

#include <boost/log/trivial.hpp>
#include <boost/algorithm/string/join.hpp>

#include <utility>

namespace gameplay
{
Material::Material(gsl::not_null<std::shared_ptr<ShaderProgram>> shaderProgram)
        : m_shaderProgram{std::move( shaderProgram )}
{
    for( const auto& u : m_shaderProgram->getHandle().getActiveUniforms() )
        m_parameters.emplace_back( std::make_shared<MaterialParameter>( u.getName() ) );
}

Material::~Material() = default;

Material::Material(const std::string& vshPath, const std::string& fshPath, const std::vector<std::string>& defines)
        : m_shaderProgram{ShaderProgram::createFromFile( vshPath, fshPath, defines )}
{
    for( const auto& u : m_shaderProgram->getHandle().getActiveUniforms() )
        m_parameters.emplace_back( std::make_shared<MaterialParameter>( u.getName() ) );
}

void Material::bind(const Node& node) const
{
    for( const auto& param : m_parameters )
    {
        const auto success = param->bind( node, m_shaderProgram );
#ifndef NDEBUG
        if( !success )
        {
            BOOST_LOG_TRIVIAL( warning ) << "Failed to bind material parameter " << param->getName();
        }
#endif
    }

    m_shaderProgram->bind();
}

gsl::not_null<std::shared_ptr<MaterialParameter>> Material::getParameter(const std::string& name) const
{
    // Search for an existing parameter with this name.
    for( const auto& param : m_parameters )
    {
        if( param->getName() == name )
        {
            return param;
        }
    }

    // Create a new parameter and store it in our list.
    auto param = std::make_shared<MaterialParameter>( name );
    m_parameters.emplace_back( param );
    return param;
}
}
