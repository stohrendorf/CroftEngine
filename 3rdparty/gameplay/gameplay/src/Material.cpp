#include "Base.h"
#include "Material.h"
#include "ShaderProgram.h"
#include "Node.h"
#include "MaterialParameter.h"

#include <boost/log/trivial.hpp>
#include <boost/algorithm/string/join.hpp>

namespace gameplay
{
    Material::Material(const std::shared_ptr<ShaderProgram>& shaderProgram)
            : m_shaderProgram{shaderProgram}
    {
        BOOST_ASSERT( shaderProgram != nullptr );

        for( const auto& u : m_shaderProgram->getHandle().getActiveUniforms() )
            m_parameters.push_back( std::make_shared<MaterialParameter>( u.getName() ) );
    }

    Material::~Material() = default;

    Material::Material(const std::string& vshPath, const std::string& fshPath, const std::vector<std::string>& defines)
            : m_shaderProgram{ShaderProgram::createFromFile( vshPath, fshPath, defines )}
    {
        if( m_shaderProgram == nullptr )
        {
            BOOST_LOG_TRIVIAL( fatal ) << "Failed to create shader. vertexShader = " << vshPath << ", fragmentShader = "
                                       << fshPath << ", defines = " << boost::algorithm::join( defines, "; " );
            BOOST_THROW_EXCEPTION( std::runtime_error( "Failed to create shader" ) );
        }

        for( const auto& u : m_shaderProgram->getHandle().getActiveUniforms() )
            m_parameters.push_back( std::make_shared<MaterialParameter>( u.getName() ) );
    }

    void Material::bind(const Node& node)
    {
        BOOST_ASSERT( m_shaderProgram != nullptr );

        for( const auto& param : m_parameters )
        {
            BOOST_ASSERT( param );
            const auto success = param->bind( node, m_shaderProgram );
#ifndef NDEBUG
            if( !success )
            {
                BOOST_LOG_TRIVIAL( warning ) << "Failed to bind material parameter " << param->getName();
            }
#endif
        }

        m_shaderProgram->bind();

        RenderState::bind();
    }

    std::shared_ptr<MaterialParameter> Material::getParameter(const std::string& name) const
    {
        // Search for an existing parameter with this name.
        for( const auto& param : m_parameters )
        {
            BOOST_ASSERT( param );
            if( param->getName() == name )
            {
                return param;
            }
        }

        // Create a new parameter and store it in our list.
        auto param = std::make_shared<MaterialParameter>( name );
        m_parameters.push_back( param );
        return param;
    }
}
