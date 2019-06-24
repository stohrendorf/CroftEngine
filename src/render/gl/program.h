#pragma once

#include "glassert.h"
#include "gsl-lite.hpp"
#include "shader.h"
#include "texture.h"

#include <glm/gtc/type_ptr.hpp>

namespace render
{
namespace gl
{
class Program : public BindableResource
{
public:
    explicit Program(const std::string& label = {})
        : BindableResource{[](const ::gl::core::SizeType n, uint32_t* handle) {
                               BOOST_ASSERT(n == 1 && handle != nullptr);
                               *handle = ::gl::createProgram();
                           },
                           ::gl::useProgram,
                           [](const ::gl::core::SizeType n, const uint32_t* handle) {
                               BOOST_ASSERT(n == 1 && handle != nullptr);
                               ::gl::deleteProgram(*handle);
                           },
                           ::gl::ObjectIdentifier::Program,
                           label}
    {
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void attach(const Shader& shader)
    {
        BOOST_ASSERT(shader.getCompileStatus());
        GL_ASSERT(::gl::attachShader(getHandle(), shader.getHandle()));
    }

    // ReSharper disable once CppMemberFunctionMayBeConst
    void link(const std::string& label = {})
    {
        GL_ASSERT(::gl::linkProgram(getHandle()));

        setLabel(::gl::ObjectIdentifier::Program, label);
    }

    bool getLinkStatus() const
    {
        int32_t success = (int32_t)::gl::Boolean::False;
        GL_ASSERT(::gl::getProgram(getHandle(), ::gl::ProgramPropertyARB::LinkStatus, &success));
        return success == (int32_t)::gl::Boolean::True;
    }

    std::string getInfoLog() const
    {
        int32_t length = 0;
        GL_ASSERT(::gl::getProgram(getHandle(), ::gl::ProgramPropertyARB::InfoLogLength, &length));
        if(length == 0)
        {
            length = 4096;
        }
        if(length > 0)
        {
            const auto infoLog = new char[length];
            GL_ASSERT(::gl::getProgramInfoLog(getHandle(), length, nullptr, infoLog));
            infoLog[length - 1] = '\0';
            std::string result = infoLog;
            delete[] infoLog;
            return result;
        }

        return {};
    }

    auto getActiveResourceCount(const ::gl::ProgramInterface what) const
    {
        int32_t n = 0;
        GL_ASSERT(::gl::getProgramInterface(getHandle(), what, ::gl::ProgramInterfacePName::ActiveResources, &n));
        return gsl::narrow<uint32_t>(n);
    }

    class Input
    {
    public:
        explicit Input(const uint32_t program, const uint32_t index)
        {
            constexpr int32_t NumProperties = 2;
            const ::gl::ProgramResourceProperty properties[NumProperties]
                = {::gl::ProgramResourceProperty::NameLength, ::gl::ProgramResourceProperty::Location};
            int32_t values[NumProperties];
            GL_ASSERT(::gl::getProgramResource(program,
                                               ::gl::ProgramInterface::ProgramInput,
                                               index,
                                               NumProperties,
                                               properties,
                                               NumProperties,
                                               nullptr,
                                               values));

            m_location = values[1];

            Expects(values[0] > 0);
            std::vector<char> nameData(values[0]);
            GL_ASSERT(::gl::getProgramResourceName(program,
                                                   ::gl::ProgramInterface::ProgramInput,
                                                   index,
                                                   gsl::narrow<::gl::core::SizeType>(nameData.size()),
                                                   nullptr,
                                                   nameData.data()));
            m_name.assign(nameData.begin(), std::prev(nameData.end()));
        }

        const std::string& getName() const noexcept
        {
            return m_name;
        }

        auto getLocation() const noexcept
        {
            return m_location;
        }

    private:
        int32_t m_size = 0;

        std::string m_name{};

        int32_t m_location = -1;
    };

    class Uniform
    {
    public:
        explicit Uniform(const uint32_t program, const uint32_t index, int32_t& samplerIndex)
            : m_program{program}
        {
            constexpr int32_t NumProperties = 2;
            const ::gl::ProgramResourceProperty properties[NumProperties]
                = {::gl::ProgramResourceProperty::NameLength, ::gl::ProgramResourceProperty::Location};
            int32_t values[NumProperties];
            GL_ASSERT(::gl::getProgramResource(program,
                                               ::gl::ProgramInterface::Uniform,
                                               index,
                                               NumProperties,
                                               properties,
                                               NumProperties,
                                               nullptr,
                                               values));

            m_location = values[1];

            Expects(values[0] > 0);
            std::vector<char> nameData(values[0]);
            GL_ASSERT(::gl::getProgramResourceName(program,
                                                   ::gl::ProgramInterface::Uniform,
                                                   index,
                                                   gsl::narrow<::gl::core::SizeType>(nameData.size()),
                                                   nullptr,
                                                   nameData.data()));
            m_name.assign(nameData.begin(), std::prev(nameData.end()));
            int32_t type;
            GL_ASSERT(::gl::getActiveUniforms(program, 1, &index, ::gl::UniformPName::UniformType, &type));
            int32_t size = -1;
            GL_ASSERT(::gl::getActiveUniforms(program, 1, &index, ::gl::UniformPName::UniformSize, &size));
            Expects(size >= 0);

            switch((::gl::UniformType)type)
            {
            case ::gl::UniformType::Sampler1d:
            case ::gl::UniformType::Sampler1dShadow:
            case ::gl::UniformType::Sampler2d:
            case ::gl::UniformType::Sampler2dShadow:
            case ::gl::UniformType::Sampler2dRect:
            case ::gl::UniformType::Sampler2dRectShadow:
            case ::gl::UniformType::Sampler3d:
            case ::gl::UniformType::SamplerCube: m_samplerIndex = samplerIndex; samplerIndex += size;
            default: break;
            }
        }

        const std::string& getName() const noexcept
        {
            return m_name;
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        template<typename T>
        void set(const T& value)
        {
            GL_ASSERT(::gl::programUniform1(m_program, m_location, value));
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        template<typename T>
        void set(const T* values, const ::gl::core::SizeType count)
        {
            BOOST_ASSERT(values != nullptr);
            GL_ASSERT(::gl::programUniform1(m_program, m_location, count, values));
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::mat3& value)
        {
            GL_ASSERT(::gl::programUniformMatrix3(m_program, m_location, 1, false, value_ptr(value)));
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::mat4& value)
        {
            GL_ASSERT(::gl::programUniformMatrix4(m_program, m_location, 1, false, value_ptr(value)));
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::mat4* values, const ::gl::core::SizeType count)
        {
            BOOST_ASSERT(values != nullptr);
            GL_ASSERT(::gl::programUniformMatrix4(
                m_program, m_location, count, false, reinterpret_cast<const float*>(values)));
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec2& value)
        {
            GL_ASSERT(::gl::programUniform2(m_program, m_location, value.x, value.y));
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec2* values, const ::gl::core::SizeType count)
        {
            BOOST_ASSERT(values != nullptr);
            GL_ASSERT(::gl::programUniform2(m_program, m_location, count, reinterpret_cast<const float*>(values)));
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec3* values, const ::gl::core::SizeType count)
        {
            BOOST_ASSERT(values != nullptr);
            GL_ASSERT(::gl::programUniform3(m_program, m_location, count, reinterpret_cast<const float*>(values)));
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec4* values, const ::gl::core::SizeType count)
        {
            BOOST_ASSERT(values != nullptr);
            GL_ASSERT(::gl::programUniform4(m_program, m_location, count, reinterpret_cast<const float*>(values)));
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec3& value)
        {
            GL_ASSERT(::gl::programUniform3(m_program, m_location, value.x, value.y, value.z));
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec4& value)
        {
            GL_ASSERT(::gl::programUniform4(m_program, m_location, value.x, value.y, value.z, value.w));
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        template<typename T>
        void set(const std::vector<T>& values)
        {
            set(values.data(), gsl::narrow<::gl::core::SizeType>(values.size()));
        }

        static ::gl::TextureUnit textureUnit(size_t n)
        {
            Expects(n >= 0 && n < 32);
            return static_cast<::gl::TextureUnit>(static_cast<::gl::core::EnumType>(::gl::TextureUnit::Texture0) + n);
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const Texture& texture)
        {
            BOOST_ASSERT(m_samplerIndex >= 0);

            GL_ASSERT(::gl::activeTexture(textureUnit(m_samplerIndex)));

            // Bind the sampler - this binds the texture and applies sampler state
            texture.bind();

            GL_ASSERT(::gl::programUniform1(m_program, m_location, m_samplerIndex));
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const std::vector<std::shared_ptr<Texture>>& values)
        {
            BOOST_ASSERT(m_samplerIndex >= 0);
            Expects(values.size() <= 32);

            // Set samplers as active and load texture unit array
            std::vector<int32_t> units;
            for(size_t i = 0; i < values.size(); ++i)
            {
                GL_ASSERT(::gl::activeTexture(textureUnit(i)));

                // Bind the sampler - this binds the texture and applies sampler state
                values[i]->bind();

                units.emplace_back(gsl::narrow<int32_t>(m_samplerIndex + i));
            }

            // Pass texture unit array to GL
            GL_ASSERT(::gl::programUniform1(
                m_program, m_location, static_cast<::gl::core::SizeType>(values.size()), units.data()));
        }

        int32_t getLocation() const noexcept
        {
            return m_location;
        }

    private:
        int32_t m_samplerIndex = -1;

        std::string m_name{};

        int32_t m_location = -1;

        const uint32_t m_program;
    };

    std::vector<Input> getInputs() const
    {
        const auto n = getActiveResourceCount(::gl::ProgramInterface::ProgramInput);

        std::vector<Input> inputs;
        inputs.reserve(n);
        for(uint32_t i = 0; i < n; ++i)
            inputs.emplace_back(getHandle(), i);
        return inputs;
    }

    std::vector<Uniform> getUniforms() const
    {
        const auto n = getActiveResourceCount(::gl::ProgramInterface::Uniform);

        std::vector<Uniform> uniforms;
        uniforms.reserve(n);
        int32_t samplerIndex = 0;
        for(uint32_t i = 0; i < n; ++i)
            uniforms.emplace_back(getHandle(), i, samplerIndex);
        return uniforms;
    }
};
} // namespace gl
} // namespace render
