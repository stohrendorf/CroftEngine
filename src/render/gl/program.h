#pragma once

#include "buffer.h"
#include "glassert.h"
#include "shader.h"
#include "texture.h"

#include <boost/assert.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <gsl-lite.hpp>

namespace render::gl
{
class Program;

template<::gl::ProgramInterface _Type>
class ProgramInterface
{
public:
  static constexpr ::gl::ProgramInterface Type = _Type;

  explicit ProgramInterface(const Program& program, uint32_t index);

  virtual ~ProgramInterface() = default;

  [[nodiscard]] const std::string& getName() const noexcept
  {
    return m_name;
  }

  [[nodiscard]] auto getIndex() const
  {
    return m_index;
  }

protected:
  static int32_t getProperty(const Program& program, uint32_t index, ::gl::ProgramResourceProperty what);

  static std::vector<int32_t>
    getProperty(const Program& program, uint32_t index, ::gl::ProgramResourceProperty what, ::gl::core::SizeType count);

private:
  std::string m_name{};
  const uint32_t m_index;
};

template<::gl::ProgramInterface _Type>
class LocatableProgramInterface : public ProgramInterface<_Type>
{
public:
  explicit LocatableProgramInterface(const Program& program, uint32_t index)
      : ProgramInterface<_Type>{program, index}
      , m_location{ProgramInterface<_Type>::getProperty(program, index, ::gl::ProgramResourceProperty::Location)}
  {
  }

  [[nodiscard]] auto getLocation() const noexcept
  {
    return m_location;
  }

private:
  const int32_t m_location;
};

using ProgramInput = LocatableProgramInterface<::gl::ProgramInterface::ProgramInput>;
using ProgramOutput = LocatableProgramInterface<::gl::ProgramInterface::ProgramOutput>;

template<::gl::ProgramInterface _Type, ::gl::BufferTargetARB _Target>
class ProgramBlock : public ProgramInterface<_Type>
{
public:
  static_assert(_Type == ::gl::ProgramInterface::UniformBlock || _Type == ::gl::ProgramInterface::ShaderStorageBlock);

  explicit ProgramBlock(const Program& program, const uint32_t index)
      : ProgramInterface<_Type>{program, index}
      , m_binding{ProgramInterface<_Type>::getProperty(program, index, ::gl::ProgramResourceProperty::BufferBinding)}
  {
    Expects(m_binding >= 0);
  }

  template<typename T>
  void bind(const Buffer<T, _Target>& buffer)
  {
    GL_ASSERT(::gl::bindBufferBase(_Target, m_binding, buffer.getHandle()));
  }

  [[nodiscard]] auto getBinding() const
  {
    return m_binding;
  }

private:
  const int32_t m_binding;
};

using ShaderStorageBlock
  = ProgramBlock<::gl::ProgramInterface::ShaderStorageBlock, ::gl::BufferTargetARB::ShaderStorageBuffer>;
using UniformBlock = ProgramBlock<::gl::ProgramInterface::UniformBlock, ::gl::BufferTargetARB::UniformBuffer>;

class Uniform : public LocatableProgramInterface<::gl::ProgramInterface::Uniform>
{
public:
  explicit Uniform(const Program& program, uint32_t index, int32_t& samplerIndex);

  // ReSharper disable once CppMemberFunctionMayBeConst
  template<typename T>
  std::enable_if_t<std::is_trivial_v<T>, void> set(const T& value)
  {
    GL_ASSERT(::gl::programUniform1(m_program, getLocation(), value));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  template<typename T>
  void set(const std::vector<T>& values)
  {
    GL_ASSERT(
      ::gl::programUniform1(m_program, getLocation(), gsl::narrow<::gl::core::SizeType>(values.size()), values.data()));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  template<typename T, size_t N>
  void set(const std::array<T, N>& values)
  {
    GL_ASSERT(
      ::gl::programUniform1(m_program, getLocation(), gsl::narrow<::gl::core::SizeType>(values.size()), values.data()));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void set(const glm::mat3& value)
  {
    GL_ASSERT(::gl::programUniformMatrix3(m_program, getLocation(), 1, false, value_ptr(value)));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void set(const glm::mat4& value)
  {
    GL_ASSERT(::gl::programUniformMatrix4(m_program, getLocation(), 1, false, value_ptr(value)));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void set(const std::vector<glm::mat4>& values)
  {
    GL_ASSERT(::gl::programUniformMatrix4(m_program,
                                          getLocation(),
                                          gsl::narrow<::gl::core::SizeType>(values.size()),
                                          false,
                                          reinterpret_cast<const float*>(values.data())));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void set(const glm::vec2& value)
  {
    GL_ASSERT(::gl::programUniform2(m_program, getLocation(), value.x, value.y));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void set(const std::vector<glm::vec2>& values)
  {
    GL_ASSERT(::gl::programUniform2(m_program,
                                    getLocation(),
                                    gsl::narrow<::gl::core::SizeType>(values.size()),
                                    reinterpret_cast<const float*>(values.data())));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void set(const std::vector<glm::vec3>& values)
  {
    GL_ASSERT(::gl::programUniform3(m_program,
                                    getLocation(),
                                    gsl::narrow<::gl::core::SizeType>(values.size()),
                                    reinterpret_cast<const float*>(values.data())));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void set(const std::vector<glm::vec4>& values)
  {
    GL_ASSERT(::gl::programUniform4(m_program,
                                    getLocation(),
                                    gsl::narrow<::gl::core::SizeType>(values.size()),
                                    reinterpret_cast<const float*>(values.data())));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void set(const glm::vec3& value)
  {
    GL_ASSERT(::gl::programUniform3(m_program, getLocation(), value.x, value.y, value.z));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void set(const glm::vec4& value)
  {
    GL_ASSERT(::gl::programUniform4(m_program, getLocation(), value.x, value.y, value.z, value.w));
  }

  static ::gl::TextureUnit textureUnit(size_t n)
  {
    Expects(n < 32);
    return static_cast<::gl::TextureUnit>(static_cast<::gl::core::EnumType>(::gl::TextureUnit::Texture0) + n);
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void set(const Texture& texture)
  {
    BOOST_ASSERT(m_samplerIndex >= 0);
    GL_ASSERT(::gl::bindTextureUnit(m_samplerIndex, texture.getHandle()));
    GL_ASSERT(::gl::programUniform1(m_program, getLocation(), m_samplerIndex));
  }

  void set(const gsl::not_null<std::shared_ptr<Texture>>& texture)
  {
    set(*texture);
  }

  template<typename _It>
  void setTextures(const _It& begin, const _It& end)
  {
    BOOST_ASSERT(m_samplerIndex >= 0);

    std::vector<int32_t> indices;
    for(auto it = begin; it != end; ++it)
    {
      const auto idx = m_samplerIndex + gsl::narrow_cast<int32_t>(indices.size());
      GL_ASSERT(::gl::bindTextureUnit(idx, (*it)->getHandle()));
      indices.emplace_back(idx);
    }

    BOOST_ASSERT(indices.size() == static_cast<size_t>(m_size));
    GL_ASSERT(::gl::programUniform1(
      m_program, getLocation(), gsl::narrow_cast<::gl::core::SizeType>(indices.size()), indices.data()));
  }

  template<typename T>
  std::enable_if_t<std::is_base_of_v<Texture, T>, void> set(const std::vector<std::shared_ptr<T>>& textures)
  {
    setTextures(textures.begin(), textures.end());
  }

  template<typename T, size_t N>
  std::enable_if_t<std::is_base_of_v<Texture, T>, void> set(const std::array<std::shared_ptr<T>, N>& textures)
  {
    setTextures(textures.begin(), textures.end());
  }

  template<typename T>
  void set(const std::shared_ptr<T>& value)
  {
    set(*value);
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void set(const std::vector<std::shared_ptr<Texture>>& textures)
  {
    BOOST_ASSERT(m_samplerIndex >= 0);
    BOOST_ASSERT(textures.size() == static_cast<size_t>(m_size));
    Expects(textures.size() <= 32);

    // Set samplers as active and load texture unit array
    std::vector<int32_t> units;
    for(size_t i = 0; i < textures.size(); ++i)
    {
      GL_ASSERT(::gl::bindTextureUnit(gsl::narrow_cast<uint32_t>(i), textures[i]->getHandle()));
      units.emplace_back(gsl::narrow<int32_t>(m_samplerIndex + i));
    }

    // Pass texture unit array to GL
    GL_ASSERT(::gl::programUniform1(
      m_program, getLocation(), static_cast<::gl::core::SizeType>(textures.size()), units.data()));
  }

private:
  int32_t m_samplerIndex = -1;
  int32_t m_size = -1;
  const uint32_t m_program;
};

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
  template<::gl::ShaderType _Type>
  void attach(const Shader<_Type>& shader)
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

  [[nodiscard]] bool getLinkStatus() const
  {
    auto success = static_cast<int32_t>(::gl::Boolean::False);
    GL_ASSERT(::gl::getProgram(getHandle(), ::gl::ProgramPropertyARB::LinkStatus, &success));
    return success == static_cast<int32_t>(::gl::Boolean::True);
  }

  [[nodiscard]] std::string getInfoLog() const
  {
    int32_t length = 0;
    GL_ASSERT(::gl::getProgram(getHandle(), ::gl::ProgramPropertyARB::InfoLogLength, &length));
    if(length == 0)
    {
      length = 4096;
    }
    if(length > 0)
    {
      const gsl::owner<gsl::zstring> infoLog = new char[length];
      GL_ASSERT(::gl::getProgramInfoLog(getHandle(), length, nullptr, infoLog));
      infoLog[length - 1] = '\0';
      std::string result = infoLog;
      delete[] infoLog;
      return result;
    }

    return {};
  }

  [[nodiscard]] auto getActiveResourceCount(const ::gl::ProgramInterface what) const
  {
    int32_t n = 0;
    GL_ASSERT(::gl::getProgramInterface(getHandle(), what, ::gl::ProgramInterfacePName::ActiveResources, &n));
    return gsl::narrow<uint32_t>(n);
  }

  [[nodiscard]] std::vector<ProgramInput> getInputs() const
  {
    return getInputs<ProgramInput>();
  }

  [[nodiscard]] std::vector<ProgramOutput> getOutputs() const
  {
    return getInputs<ProgramOutput>();
  }

  [[nodiscard]] std::vector<Uniform> getUniforms(int32_t& samplerIndex) const
  {
    return getInputs<Uniform>(samplerIndex);
  }

  [[nodiscard]] std::vector<ShaderStorageBlock> getShaderStorageBlocks() const
  {
    return getInputs<ShaderStorageBlock>();
  }

  [[nodiscard]] std::vector<UniformBlock> getUniformBlocks(int32_t& samplerIndex) const
  {
    return getInputs<UniformBlock>();
  }

private:
  template<typename T, typename... Args>
  [[nodiscard]] std::vector<T> getInputs(Args&&... args) const
  {
    const auto n = getActiveResourceCount(T::Type);
    std::vector<T> inputs;
    inputs.reserve(n);
    for(uint32_t i = 0; i < n; ++i)
    {
      inputs.emplace_back(*this, i, std::forward<Args>(args)...);
    }
    return inputs;
  }
};

template<::gl::ProgramInterface _Type>
inline ProgramInterface<_Type>::ProgramInterface(const Program& program, const uint32_t index)
    : m_index{index}
{
  const auto nameLength = getProperty(program, index, ::gl::ProgramResourceProperty::NameLength);
  Expects(nameLength > 0);
  std::vector<char> nameData(nameLength, 0);
  GL_ASSERT(::gl::getProgramResourceName(
    program.getHandle(), Type, index, gsl::narrow<::gl::core::SizeType>(nameData.size()), nullptr, nameData.data()));
  m_name = nameData.data();
}

template<::gl::ProgramInterface _Type>
inline int32_t
  ProgramInterface<_Type>::getProperty(const Program& program, const uint32_t index, ::gl::ProgramResourceProperty what)
{
  constexpr int32_t NumProperties = 1;
  const ::gl::ProgramResourceProperty properties[NumProperties] = {what};
  int32_t values[1];
  GL_ASSERT(::gl::getProgramResource(program.getHandle(),
                                     Type,
                                     index,
                                     NumProperties,
                                     properties,
                                     gsl::narrow_cast<int>(std::size(values)),
                                     nullptr,
                                     values));
  return values[0];
}

template<::gl::ProgramInterface _Type>
inline std::vector<int32_t> ProgramInterface<_Type>::getProperty(const Program& program,
                                                                 const uint32_t index,
                                                                 ::gl::ProgramResourceProperty what,
                                                                 ::gl::core::SizeType count)
{
  constexpr int32_t NumProperties = 1;
  const ::gl::ProgramResourceProperty properties[NumProperties] = {what};
  Expects(count >= 0);
  std::vector<int32_t> values(count, 0);
  GL_ASSERT(::gl::getProgramResource(
    program.getHandle(), Type, index, NumProperties, properties, count, nullptr, values.data()));
  return values;
}

inline Uniform::Uniform(const Program& program, const uint32_t index, int32_t& samplerIndex)
    : LocatableProgramInterface{program, index}
    , m_program{program.getHandle()}
{
  int32_t type;
  GL_ASSERT(::gl::getActiveUniforms(program.getHandle(), 1, &index, ::gl::UniformPName::UniformType, &type));
  GL_ASSERT(::gl::getActiveUniforms(program.getHandle(), 1, &index, ::gl::UniformPName::UniformSize, &m_size));
  Expects(m_size >= 0);

  switch(static_cast<::gl::UniformType>(type))
  {
  case ::gl::UniformType::Sampler1d:
  case ::gl::UniformType::Sampler1dShadow:
  case ::gl::UniformType::Sampler1dArray:
  case ::gl::UniformType::Sampler1dArrayShadow:
  case ::gl::UniformType::Sampler2d:
  case ::gl::UniformType::Sampler2dShadow:
  case ::gl::UniformType::Sampler2dArray:
  case ::gl::UniformType::Sampler2dArrayShadow:
  case ::gl::UniformType::Sampler2dRect:
  case ::gl::UniformType::Sampler2dRectShadow:
  case ::gl::UniformType::Sampler2dMultisample:
  case ::gl::UniformType::Sampler2dMultisampleArray:
  case ::gl::UniformType::Sampler3d:
  case ::gl::UniformType::SamplerCube:
  case ::gl::UniformType::SamplerCubeShadow:
  case ::gl::UniformType::SamplerCubeMapArray:
  case ::gl::UniformType::SamplerCubeMapArrayShadow: m_samplerIndex = samplerIndex; samplerIndex += m_size;
  default: break;
  }
}
} // namespace render::gl
