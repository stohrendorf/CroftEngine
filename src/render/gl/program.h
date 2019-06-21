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

    auto getActiveAttributeCount() const
    {
        int32_t activeAttributes = 0;
        GL_ASSERT(::gl::getProgram(getHandle(), ::gl::ProgramPropertyARB::ActiveAttributes, &activeAttributes));
        return activeAttributes;
    }

    auto getActiveAttributeMaxLength() const
    {
        int32_t length = 0;
        GL_ASSERT(::gl::getProgram(getHandle(), ::gl::ProgramPropertyARB::ActiveAttributeMaxLength, &length));
        return length;
    }

    auto getActiveUniformCount() const
    {
        int32_t activeAttributes = 0;
        GL_ASSERT(::gl::getProgram(getHandle(), ::gl::ProgramPropertyARB::ActiveUniforms, &activeAttributes));
        return gsl::narrow<uint32_t>(activeAttributes);
    }

    auto getActiveUniformMaxLength() const
    {
        int32_t length = 0;
        GL_ASSERT(::gl::getProgram(getHandle(), ::gl::ProgramPropertyARB::ActiveUniformMaxLength, &length));
        return length;
    }

    class ActiveAttribute
    {
    public:
        explicit ActiveAttribute(const uint32_t program, const uint32_t index, const int32_t maxLength)
        {
            Expects(maxLength >= 0);
            auto* attribName = new char[gsl::narrow_cast<size_t>(maxLength) + 1];
            GL_ASSERT(::gl::getActiveAttri(program, index, maxLength, nullptr, &m_size, &m_type, attribName));
            attribName[maxLength] = '\0';
            m_name = attribName;
            delete[] attribName;

            m_location = GL_ASSERT_FN(::gl::getAttribLocation(program, m_name.c_str()));
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

        ::gl::AttributeType m_type{};

        std::string m_name{};

        int32_t m_location = -1;
    };

    class ActiveUniform
    {
    public:
        explicit ActiveUniform(const uint32_t program,
                               const uint32_t index,
                               const int32_t maxLength,
                               int32_t& samplerIndex)
            : m_program{program}
        {
            auto* uniformName = new char[maxLength + 1];
            GL_ASSERT(::gl::getActiveUniform(program, index, maxLength, nullptr, &m_size, &m_type, uniformName));
            uniformName[maxLength] = '\0';

            m_name = uniformName;
            delete[] uniformName;

            m_location = GL_ASSERT_FN(::gl::getUniformLocation(program, m_name.c_str()));

            switch(m_type)
            {
            case ::gl::AttributeType::Sampler1d:
            case ::gl::AttributeType::Sampler1dShadow:
            case ::gl::AttributeType::Sampler2d:
            case ::gl::AttributeType::Sampler2dShadow:
            case ::gl::AttributeType::Sampler2dRect:
            case ::gl::AttributeType::Sampler2dRectShadow:
            case ::gl::AttributeType::Sampler3d:
            case ::gl::AttributeType::SamplerCube: m_samplerIndex = samplerIndex; samplerIndex += m_size;
            default: break;
            }
        }

        const std::string& getName() const noexcept
        {
            return m_name;
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const float value)
        {
            GL_ASSERT(::gl::programUniform1(m_program, m_location, value));
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const float* values, const ::gl::core::SizeType count)
        {
            BOOST_ASSERT(values != nullptr);
            GL_ASSERT(::gl::programUniform1(m_program, m_location, count, values));
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const int32_t value)
        {
            GL_ASSERT(::gl::programUniform1(m_program, m_location, value));
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const int32_t* values, const ::gl::core::SizeType count)
        {
            BOOST_ASSERT(values != nullptr);
            GL_ASSERT(::gl::programUniform1(m_program, m_location, count, values));
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
        void set(const glm::vec3& value)
        {
            GL_ASSERT(::gl::programUniform3(m_program, m_location, value.x, value.y, value.z));
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec3* values, const ::gl::core::SizeType count)
        {
            BOOST_ASSERT(values != nullptr);
            GL_ASSERT(::gl::programUniform3(m_program, m_location, count, reinterpret_cast<const float*>(values)));
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const std::vector<glm::vec3>& values)
        {
            set(values.data(), gsl::narrow<::gl::core::SizeType>(values.size()));
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec4& value)
        {
            GL_ASSERT(::gl::programUniform4(m_program, m_location, value.x, value.y, value.z, value.w));
        }

        // ReSharper disable once CppMemberFunctionMayBeConst
        void set(const glm::vec4* values, const ::gl::core::SizeType count)
        {
            BOOST_ASSERT(values != nullptr);
            GL_ASSERT(::gl::programUniform4(m_program, m_location, count, reinterpret_cast<const float*>(values)));
        }

        static const ::gl::TextureUnit textureUnit(int32_t n)
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
            for(int32_t i = 0; i < values.size(); ++i)
            {
                GL_ASSERT(::gl::activeTexture(textureUnit(i)));

                // Bind the sampler - this binds the texture and applies sampler state
                values[i]->bind();

                units.emplace_back(m_samplerIndex + i);
            }

            // Pass texture unit array to GL
            GL_ASSERT(::gl::programUniform1(
                m_program, m_location, static_cast<::gl::core::SizeType>(values.size()), units.data()));
        }

        auto getType() const noexcept
        {
            return m_type;
        }

        int32_t getLocation() const noexcept
        {
            return m_location;
        }

        int32_t getSamplerIndex() const noexcept
        {
            return m_samplerIndex;
        }

    private:
        int32_t m_size = 0;

        ::gl::AttributeType m_type{};

        std::string m_name{};

        int32_t m_location = -1;

        int32_t m_samplerIndex = -1;

        const uint32_t m_program;
    };

    ActiveAttribute getActiveAttribute(const uint32_t index) const
    {
        return ActiveAttribute{getHandle(), index, getActiveAttributeMaxLength()};
    }

    std::vector<ActiveAttribute> getActiveAttributes() const
    {
        std::vector<ActiveAttribute> attribs;
        auto count = getActiveAttributeCount();
        const auto maxLength = getActiveAttributeMaxLength();
        for(decltype(count) i = 0; i < count; ++i)
            attribs.emplace_back(getHandle(), i, maxLength);
        return attribs;
    }

    ActiveUniform getActiveUniform(const uint32_t index, int32_t& samplerIndex) const
    {
        return ActiveUniform{getHandle(), index, getActiveUniformMaxLength(), samplerIndex};
    }

    std::vector<ActiveUniform> getActiveUniforms() const
    {
        std::vector<ActiveUniform> uniforms;
        auto count = getActiveUniformCount();
        const auto maxLength = getActiveUniformMaxLength();
        int32_t samplerIndex = 0;
        for(decltype(count) i = 0; i < count; ++i)
            uniforms.emplace_back(getHandle(), i, maxLength, samplerIndex);
        return uniforms;
    }
};
} // namespace gl
} // namespace render
