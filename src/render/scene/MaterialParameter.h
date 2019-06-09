#pragma once

#include "ShaderProgram.h"

#include "gsl-lite.hpp"

#include <glm/glm.hpp>
#include <boost/optional.hpp>

namespace render
{
namespace scene
{
class Node;


class MaterialParameter
{
public:
    explicit MaterialParameter(std::string name);

    ~MaterialParameter();

    MaterialParameter(const MaterialParameter&&) = delete;

    MaterialParameter& operator=(const MaterialParameter&) = delete;

    MaterialParameter& operator=(MaterialParameter&&) = delete;

    const std::string& getName() const;

    void set(float value);

    void set(int value);

    void set(const float* values, std::size_t count = 1);

    void set(const int* values, std::size_t count = 1);

    void set(const glm::vec2& value);

    void set(const glm::vec2* values, std::size_t count = 1);

    void set(const glm::vec3& value);

    void set(const glm::vec3* values, std::size_t count = 1);

    void set(const glm::vec4& value);

    void set(const glm::vec4* values, std::size_t count = 1);

    void set(const glm::mat4& value);

    void set(const glm::mat4* values, std::size_t count = 1);

    void set(const std::shared_ptr<gl::Texture>& texture);

    void set(const std::vector<std::shared_ptr<gl::Texture>>& textures);

    template<typename T>
    void set(const std::vector<T>& values)
    {
        set( values.data(), values.size() );
    }

    template<class ClassType, class ValueType>
    void bind(ClassType* classInstance, ValueType (ClassType::*valueMethod)() const)
    {
        m_valueSetter = [classInstance, valueMethod](const Node& /*node*/, gl::Program::ActiveUniform& uniform) {
            uniform.set( (classInstance->*valueMethod)() );
        };
    }

    using UniformValueSetter = void(const Node& node, gl::Program::ActiveUniform& uniform);

    void bind(std::function<UniformValueSetter>&& setter)
    {
        m_valueSetter = std::move( setter );
    }

    template<class ClassType, class ValueType>
    void bind(ClassType* classInstance, ValueType (ClassType::*valueMethod)() const,
              std::size_t (ClassType::*countMethod)() const)
    {
        m_valueSetter = [classInstance, valueMethod, countMethod](const Node& /*node*/,
                                                                  const gl::Program::ActiveUniform& uniform) {
            uniform.set( (classInstance->*valueMethod)(), (classInstance->*countMethod)() );
        };
    }

    void bindModelMatrix();

    void bindViewMatrix();

    void bindModelViewMatrix();

    void bindProjectionMatrix();

    bool bind(const Node& node, const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram);

private:
    gl::Program::ActiveUniform* getUniform(const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram);

    const std::string m_name;

    boost::optional<std::function<UniformValueSetter>> m_valueSetter;
};
}
}
