#include "uniformparameter.h"

#include "Node.h"

#include <boost/log/trivial.hpp>
#include <utility>

namespace render
{
namespace scene
{
bool UniformParameter::bind(const Node& node, const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram)
{
    const auto setter = node.findUniformSetter(m_name);
    if(!m_valueSetter && setter == nullptr)
    {
        // don't have an explicit setter present on material or node level, assuming it's set on shader level
        return true;
    }

    const auto uniform = findUniform(shaderProgram);
    if(uniform == nullptr)
        return false;

    if(setter != nullptr)
        (*setter)(node, *uniform);
    else
        (*m_valueSetter)(node, *uniform);

    return true;
}

void UniformParameter::bindModelMatrix()
{
    m_valueSetter = [](const Node& node, gl::ProgramUniform& uniform) { uniform.set(node.getModelMatrix()); };
}

void UniformParameter::bindViewMatrix()
{
    m_valueSetter = [](const Node& node, gl::ProgramUniform& uniform) { uniform.set(node.getViewMatrix()); };
}

void UniformParameter::bindModelViewMatrix()
{
    m_valueSetter = [](const Node& node, gl::ProgramUniform& uniform) { uniform.set(node.getModelViewMatrix()); };
}

void UniformParameter::bindProjectionMatrix()
{
    m_valueSetter = [](const Node& node, gl::ProgramUniform& uniform) { uniform.set(node.getProjectionMatrix()); };
}
} // namespace scene
} // namespace render
