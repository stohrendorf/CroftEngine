#include "material.h"

#include "names.h"
#include "node.h"
#include "shaderprogram.h"
#include "uniformparameter.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/log/trivial.hpp>
#include <utility>

namespace render
{
namespace scene
{
Material::Material(gsl::not_null<std::shared_ptr<ShaderProgram>> shaderProgram)
    : m_shaderProgram{std::move(shaderProgram)}
{
    for(const auto& u : m_shaderProgram->getHandle().getUniforms())
        m_uniforms.emplace_back(std::make_shared<UniformParameter>(u.getName()));
    for(const auto& u : m_shaderProgram->getHandle().getShaderStorageBlocks())
        m_buffers.emplace_back(std::make_shared<BufferParameter>(u.getName()));
}

Material::~Material() = default;

Material::Material(const std::string& vshPath, const std::string& fshPath, const std::vector<std::string>& defines)
    : Material{ShaderProgram::createFromFile(vshPath, fshPath, defines)}
{
}

void Material::bind(const Node& node) const
{
    for(const auto& param : m_uniforms)
    {
        const auto success = param->bind(node, m_shaderProgram);
#ifndef NDEBUG
        if(!success)
        {
            BOOST_LOG_TRIVIAL(warning) << "Failed to bind material uniform " << param->getName();
        }
#endif
    }

    for(const auto& param : m_buffers)
    {
        const auto success = param->bind(node, m_shaderProgram);
#ifndef NDEBUG
        if(!success)
        {
            BOOST_LOG_TRIVIAL(warning) << "Failed to bind material buffer " << param->getName();
        }
#endif
    }

    m_shaderProgram->bind();
}

gsl::not_null<std::shared_ptr<UniformParameter>> Material::getUniform(const std::string& name) const
{
    // Search for an existing parameter with this name.
    for(const auto& param : m_uniforms)
    {
        if(param->getName() == name)
        {
            return param;
        }
    }

    // Create a new parameter and store it in our list.
    auto param = std::make_shared<UniformParameter>(name);
    m_uniforms.emplace_back(param);
    return param;
}

gsl::not_null<std::shared_ptr<BufferParameter>> Material::getBuffer(const std::string& name) const
{
    // Search for an existing parameter with this name.
    for(const auto& param : m_buffers)
    {
        if(param->getName() == name)
        {
            return param;
        }
    }

    // Create a new parameter and store it in our list.
    auto param = std::make_shared<BufferParameter>(name);
    m_buffers.emplace_back(param);
    return param;
}
} // namespace scene
} // namespace render
