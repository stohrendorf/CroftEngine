#pragma once

#include "buffer.h"
#include "glassert.h"
#include "shader.h"
#include "texture.h"

#include <boost/assert.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <gsl-lite.hpp>

namespace gl
{
class Program;

// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<api::ProgramInterface _Type>
class ProgramInterface
{
public:
  static constexpr api::ProgramInterface Type = _Type;

  explicit ProgramInterface(const Program& program, uint32_t index);
  virtual ~ProgramInterface() = default;

  explicit ProgramInterface(ProgramInterface<_Type>&& rhs)
      : m_index{std::exchange(rhs.m_index, InvalidIndex)}
  {
  }

  ProgramInterface<_Type>& operator=(ProgramInterface<_Type>&& rhs)
  {
    m_index = std::exchange(rhs.m_index, InvalidIndex);
    return *this;
  }

  [[nodiscard]] const std::string& getName() const noexcept
  {
    return m_name;
  }

  [[nodiscard]] auto getIndex() const
  {
    return m_index;
  }

protected:
  static int32_t getProperty(const Program& program, uint32_t index, api::ProgramResourceProperty what);

  static std::vector<int32_t>
    getProperty(const Program& program, uint32_t index, api::ProgramResourceProperty what, api::core::SizeType count);

private:
  std::string m_name{};
  uint32_t m_index;
  static constexpr uint32_t InvalidIndex = std::numeric_limits<uint32_t>::max();
};

// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<api::ProgramInterface _Type>
class LocatableProgramInterface : public ProgramInterface<_Type>
{
public:
  explicit LocatableProgramInterface(const Program& program, uint32_t index)
      : ProgramInterface<_Type>{program, index}
      , m_location{ProgramInterface<_Type>::getProperty(program, index, api::ProgramResourceProperty::Location)}
  {
  }

  explicit LocatableProgramInterface(LocatableProgramInterface<_Type>&& rhs)
      : ProgramInterface<_Type>{std::move(rhs)}
      , m_location{std::exchange(rhs.m_location, -1)}
  {
  }

  LocatableProgramInterface<_Type>& operator=(LocatableProgramInterface<_Type>&& rhs)
  {
    ProgramInterface<_Type>::operator=(std::move(rhs));
    m_location = std::exchange(rhs.m_location, -1);
    return *this;
  }

  [[nodiscard]] auto getLocation() const noexcept
  {
    return m_location;
  }

private:
  int32_t m_location;
};

using ProgramInput = LocatableProgramInterface<api::ProgramInterface::ProgramInput>;
using ProgramOutput = LocatableProgramInterface<api::ProgramInterface::ProgramOutput>;

// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<api::ProgramInterface _Type, api::BufferTargetARB _Target>
class ProgramBlock final : public ProgramInterface<_Type>
{
public:
  static_assert(_Type == api::ProgramInterface::UniformBlock || _Type == api::ProgramInterface::ShaderStorageBlock);

  explicit ProgramBlock(const Program& program, const uint32_t index)
      : ProgramInterface<_Type>{program, index}
      , m_binding{ProgramInterface<_Type>::getProperty(program, index, api::ProgramResourceProperty::BufferBinding)}
  {
    Expects(m_binding >= 0);
  }

  explicit ProgramBlock(ProgramBlock<_Type, _Target>&& rhs)
      : ProgramInterface<_Type>{std::move(rhs)}
      , m_binding{std::exchange(rhs.m_binding, -1)}
  {
  }

  ProgramBlock<_Type, _Target>& operator=(ProgramBlock<_Type, _Target>&& rhs)
  {
    ProgramInterface<_Type>::operator=(std::move(rhs));
    m_binding = std::exchange(rhs.m_binding, -1);
    return *this;
  }

  template<typename T>
  void bind(const Buffer<T, _Target>& buffer)
  {
    Expects(m_binding >= 0);
    GL_ASSERT(api::bindBufferBase(_Target, m_binding, buffer.getHandle()));
  }

  [[nodiscard]] auto getBinding() const
  {
    return m_binding;
  }

private:
  int32_t m_binding;
};

using ShaderStorageBlock
  = ProgramBlock<api::ProgramInterface::ShaderStorageBlock, api::BufferTargetARB::ShaderStorageBuffer>;
using UniformBlock = ProgramBlock<api::ProgramInterface::UniformBlock, api::BufferTargetARB::UniformBuffer>;

class Uniform final : public LocatableProgramInterface<api::ProgramInterface::Uniform>
{
public:
  explicit Uniform(const Program& program, uint32_t index, int32_t& samplerIndex);

  explicit Uniform(Uniform&& rhs)
      : LocatableProgramInterface{std::move(rhs)}
      , m_samplerIndex{std::exchange(rhs.m_samplerIndex, -1)}
      , m_size{std::exchange(rhs.m_size, -1)}
      , m_program{std::exchange(rhs.m_program, InvalidProgram)}
  {
  }

  Uniform& operator=(Uniform&& rhs)
  {
    LocatableProgramInterface::operator=(std::move(rhs));
    m_samplerIndex = std::exchange(rhs.m_samplerIndex, -1);
    m_size = std::exchange(rhs.m_size, -1);
    m_program = std::exchange(rhs.m_program, InvalidProgram);
    return *this;
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  template<typename T>
  std::enable_if_t<std::is_trivial_v<T>, void> set(const T& value)
  {
    Expects(m_program != InvalidProgram);
    GL_ASSERT(api::programUniform1(m_program, getLocation(), value));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  template<typename T>
  void set(const std::vector<T>& values)
  {
    Expects(m_program != InvalidProgram);
    GL_ASSERT(
      api::programUniform1(m_program, getLocation(), gsl::narrow<api::core::SizeType>(values.size()), values.data()));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  template<typename T, size_t N>
  void set(const std::array<T, N>& values)
  {
    Expects(m_program != InvalidProgram);
    GL_ASSERT(
      api::programUniform1(m_program, getLocation(), gsl::narrow<api::core::SizeType>(values.size()), values.data()));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void set(const glm::mat3& value)
  {
    Expects(m_program != InvalidProgram);
    GL_ASSERT(api::programUniformMatrix3(m_program, getLocation(), 1, false, value_ptr(value)));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void set(const glm::mat4& value)
  {
    Expects(m_program != InvalidProgram);
    GL_ASSERT(api::programUniformMatrix4(m_program, getLocation(), 1, false, value_ptr(value)));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void set(const std::vector<glm::mat4>& values)
  {
    Expects(m_program != InvalidProgram);
    GL_ASSERT(api::programUniformMatrix4(
      m_program,
      getLocation(),
      gsl::narrow<api::core::SizeType>(values.size()),
      false,
      reinterpret_cast<const float*>(values.data()) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
      ));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void set(const glm::vec2& value)
  {
    Expects(m_program != InvalidProgram);
    GL_ASSERT(api::programUniform2(m_program, getLocation(), value.x, value.y));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void set(const std::vector<glm::vec2>& values)
  {
    Expects(m_program != InvalidProgram);
    GL_ASSERT(api::programUniform2(
      m_program,
      getLocation(),
      gsl::narrow<api::core::SizeType>(values.size()),
      reinterpret_cast<const float*>(values.data()) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
      ));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void set(const std::vector<glm::vec3>& values)
  {
    Expects(m_program != InvalidProgram);
    GL_ASSERT(api::programUniform3(
      m_program,
      getLocation(),
      gsl::narrow<api::core::SizeType>(values.size()),
      reinterpret_cast<const float*>(values.data()) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
      ));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void set(const std::vector<glm::vec4>& values)
  {
    Expects(m_program != InvalidProgram);
    GL_ASSERT(api::programUniform4(
      m_program,
      getLocation(),
      gsl::narrow<api::core::SizeType>(values.size()),
      reinterpret_cast<const float*>(values.data()) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
      ));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void set(const glm::vec3& value)
  {
    Expects(m_program != InvalidProgram);
    GL_ASSERT(api::programUniform3(m_program, getLocation(), value.x, value.y, value.z));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void set(const glm::vec4& value)
  {
    Expects(m_program != InvalidProgram);
    GL_ASSERT(api::programUniform4(m_program, getLocation(), value.x, value.y, value.z, value.w));
  }

  static api::TextureUnit textureUnit(const size_t n)
  {
    Expects(n < api::TextureUnitCount);
    return static_cast<api::TextureUnit>(static_cast<api::core::EnumType>(api::TextureUnit::Texture0) + n);
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void set(const Texture& texture)
  {
    Expects(m_program != InvalidProgram);
    Expects(m_samplerIndex >= 0);
    GL_ASSERT(api::bindTextureUnit(m_samplerIndex, texture.getHandle()));
    GL_ASSERT(api::programUniform1(m_program, getLocation(), m_samplerIndex));
  }

  void set(const gsl::not_null<std::shared_ptr<Texture>>& texture)
  {
    set(*texture);
  }

  template<typename _It> // NOLINT(bugprone-reserved-identifier)
  void setTextures(const _It& begin, const _It& end)
  {
    Expects(m_program != InvalidProgram && m_samplerIndex >= 0 && m_size >= 0);

    std::vector<int32_t> indices;
    for(auto it = begin; it != end; ++it)
    {
      const auto idx = m_samplerIndex + gsl::narrow_cast<int32_t>(indices.size());
      GL_ASSERT(api::bindTextureUnit(idx, (*it)->getHandle()));
      indices.emplace_back(idx);
    }

    Expects(indices.size() == static_cast<size_t>(m_size));
    GL_ASSERT(api::programUniform1(
      m_program, getLocation(), gsl::narrow_cast<api::core::SizeType>(indices.size()), indices.data()));
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
    Expects(m_samplerIndex >= 0 && m_size >= 0 && textures.size() == static_cast<size_t>(m_size)
            && textures.size() <= api::TextureUnitCount);

    // Set samplers as active and load texture unit array
    std::vector<int32_t> units;
    for(size_t i = 0; i < textures.size(); ++i)
    {
      GL_ASSERT(api::bindTextureUnit(gsl::narrow_cast<uint32_t>(i), textures[i]->getHandle()));
      units.emplace_back(gsl::narrow<int32_t>(m_samplerIndex + i));
    }

    // Pass texture unit array to GL
    GL_ASSERT(
      api::programUniform1(m_program, getLocation(), static_cast<api::core::SizeType>(textures.size()), units.data()));
  }

private:
  static constexpr uint32_t InvalidProgram = std::numeric_limits<uint32_t>::max();

  int32_t m_samplerIndex = -1;
  int32_t m_size = -1;
  uint32_t m_program;
};

class Program final : public BindableResource
{
public:
  explicit Program(const std::string& label = {})
      : BindableResource{[]([[maybe_unused]] const api::core::SizeType n, uint32_t* handle) {
                           BOOST_ASSERT(n == 1 && handle != nullptr);
                           *handle = api::createProgram();
                         },
                         api::useProgram,
                         []([[maybe_unused]] const api::core::SizeType n, const uint32_t* handle) {
                           BOOST_ASSERT(n == 1 && handle != nullptr);
                           api::deleteProgram(*handle);
                         },
                         api::ObjectIdentifier::Program,
                         label}
  {
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  template<api::ShaderType _Type> // NOLINT(bugprone-reserved-identifier)
  void attach(const Shader<_Type>& shader)
  {
    BOOST_ASSERT(shader.getCompileStatus());
    GL_ASSERT(api::attachShader(getHandle(), shader.getHandle()));
  }

  // ReSharper disable once CppMemberFunctionMayBeConst
  void link(const std::string& label = {})
  {
    GL_ASSERT(api::linkProgram(getHandle()));

    setLabel(api::ObjectIdentifier::Program, label);
  }

  [[nodiscard]] bool getLinkStatus() const
  {
    auto success = static_cast<int32_t>(api::Boolean::False);
    GL_ASSERT(api::getProgram(getHandle(), api::ProgramPropertyARB::LinkStatus, &success));
    return success == static_cast<int32_t>(api::Boolean::True);
  }

  [[nodiscard]] std::string getInfoLog() const
  {
    int32_t length = 0;
    GL_ASSERT(api::getProgram(getHandle(), api::ProgramPropertyARB::InfoLogLength, &length));
    if(length == 0)
    {
      length = 4096;
    }
    if(length > 0)
    {
      std::vector<char> infoLog;
      infoLog.resize(length, '\0');
      GL_ASSERT(api::getProgramInfoLog(getHandle(), length, nullptr, infoLog.data()));
      infoLog[length - 1] = '\0';
      std::string result = infoLog.data();
      return result;
    }

    return {};
  }

  [[nodiscard]] auto getActiveResourceCount(const api::ProgramInterface what) const
  {
    int32_t n = 0;
    GL_ASSERT(api::getProgramInterface(getHandle(), what, api::ProgramInterfacePName::ActiveResources, &n));
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

  [[nodiscard]] std::vector<UniformBlock> getUniformBlocks(int32_t& /*samplerIndex*/) const
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

// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<api::ProgramInterface _Type>
ProgramInterface<_Type>::ProgramInterface(const Program& program, const uint32_t index)
    : m_index{index}
{
  const auto nameLength = getProperty(program, index, api::ProgramResourceProperty::NameLength);
  Expects(nameLength > 0);
  std::vector<char> nameData(nameLength, 0);
  GL_ASSERT(api::getProgramResourceName(
    program.getHandle(), Type, index, gsl::narrow<api::core::SizeType>(nameData.size()), nullptr, nameData.data()));
  m_name = nameData.data();
}

// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<api::ProgramInterface _Type>
int32_t ProgramInterface<_Type>::getProperty(const Program& program,
                                             const uint32_t index,
                                             const api::ProgramResourceProperty what)
{
  constexpr int32_t NumProperties = 1;
  const std::array<api::ProgramResourceProperty, NumProperties> properties{what};
  std::array<int32_t, 1> values{};
  GL_ASSERT(api::getProgramResource(program.getHandle(),
                                    Type,
                                    index,
                                    NumProperties,
                                    properties.data(),
                                    gsl::narrow_cast<api::core::SizeType>(values.size()),
                                    nullptr,
                                    values.data()));
  return values[0];
}

// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<api::ProgramInterface _Type>
std::vector<int32_t> ProgramInterface<_Type>::getProperty(const Program& program,
                                                          const uint32_t index,
                                                          const api::ProgramResourceProperty what,
                                                          const api::core::SizeType count)
{
  constexpr int32_t NumProperties = 1;
  const std::array<api::ProgramResourceProperty, NumProperties> properties{what};
  Expects(count >= 0);
  std::vector<int32_t> values(count, 0);
  GL_ASSERT(api::getProgramResource(
    program.getHandle(), Type, index, NumProperties, properties.data(), count, nullptr, values.data()));
  return values;
}

inline Uniform::Uniform(const Program& program, const uint32_t index, int32_t& samplerIndex)
    : LocatableProgramInterface{program, index}
    , m_program{program.getHandle()}
{
  int32_t type;
  GL_ASSERT(api::getActiveUniforms(program.getHandle(), 1, &index, api::UniformPName::UniformType, &type));
  GL_ASSERT(api::getActiveUniforms(program.getHandle(), 1, &index, api::UniformPName::UniformSize, &m_size));
  Expects(m_size >= 0);

  switch(static_cast<api::UniformType>(type))
  {
  case api::UniformType::Sampler1d:
  case api::UniformType::Sampler1dShadow:
  case api::UniformType::Sampler1dArray:
  case api::UniformType::Sampler1dArrayShadow:
  case api::UniformType::Sampler2d:
  case api::UniformType::Sampler2dShadow:
  case api::UniformType::Sampler2dArray:
  case api::UniformType::Sampler2dArrayShadow:
  case api::UniformType::Sampler2dRect:
  case api::UniformType::Sampler2dRectShadow:
  case api::UniformType::Sampler2dMultisample:
  case api::UniformType::Sampler2dMultisampleArray:
  case api::UniformType::Sampler3d:
  case api::UniformType::SamplerCube:
  case api::UniformType::SamplerCubeShadow:
  case api::UniformType::SamplerCubeMapArray:
  case api::UniformType::SamplerCubeMapArrayShadow: m_samplerIndex = samplerIndex; samplerIndex += m_size;
  default: break;
  }
}
} // namespace gl
