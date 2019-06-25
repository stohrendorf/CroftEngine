#pragma once

#include "ShaderProgram.h"
#include "gsl-lite.hpp"

#include <boost/optional.hpp>
#include <glm/glm.hpp>

namespace render
{
namespace scene
{
class Node;

class MaterialParameter
{
public:
    explicit MaterialParameter(std::string name)
        : m_name{std::move(name)}
    {
    }

    ~MaterialParameter() = default;

    MaterialParameter(const MaterialParameter&&) = delete;

    MaterialParameter& operator=(const MaterialParameter&) = delete;

    MaterialParameter& operator=(MaterialParameter&&) = delete;

    const std::string& getName() const
    {
        return m_name;
    }

    template<typename T>
    void set(const T& value)
    {
        m_valueSetter = [value](const Node& /*node*/, gl::ProgramUniform& uniform) { uniform.set(value); };
    }

    template<class ClassType, class ValueType>
    void bind(ClassType* classInstance, ValueType (ClassType::*valueMethod)() const)
    {
        m_valueSetter = [classInstance, valueMethod](const Node& /*node*/, gl::Program::ProgramUniform& uniform) {
            uniform.set((classInstance->*valueMethod)());
        };
    }

    using UniformValueSetter = void(const Node& node, gl::ProgramUniform& uniform);

    void bind(std::function<UniformValueSetter>&& setter)
    {
        m_valueSetter = std::move(setter);
    }

    template<class ClassType, class ValueType>
    void bind(ClassType* classInstance,
              ValueType (ClassType::*valueMethod)() const,
              std::size_t (ClassType::*countMethod)() const)
    {
        m_valueSetter = [classInstance, valueMethod, countMethod](const Node& /*node*/,
                                                                  const gl::Program::ProgramUniform& uniform) {
            uniform.set((classInstance->*valueMethod)(), (classInstance->*countMethod)());
        };
    }

    void bindModelMatrix();

    void bindViewMatrix();

    void bindModelViewMatrix();

    void bindProjectionMatrix();

    bool bind(const Node& node, const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram);

private:
    gl::ProgramUniform* findUniform(const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram) const
    {
        if(const auto uniform = shaderProgram->findUniform(m_name))
            return uniform;

        // This parameter was not found in the specified effect, so do nothing.
        BOOST_LOG_TRIVIAL(warning) << "Uniform '" << m_name << "' not found in program '" << shaderProgram->getId()
                                   << "'";

        return nullptr;
    }

    const std::string m_name;

    boost::optional<std::function<UniformValueSetter>> m_valueSetter;
};
} // namespace scene
} // namespace render
