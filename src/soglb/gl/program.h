#pragma once

#include "buffer.h"
#include "shader.h"
#include "texturehandle.h"

#include <type_traits>

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

  ProgramInterface(ProgramInterface<_Type>&& rhs) noexcept
      : m_name{std::move(rhs.m_name)}
  {
  }

  ProgramInterface<_Type>& operator=(ProgramInterface<_Type>&& rhs) noexcept
  {
    m_name = std::move(rhs.m_name);
    return *this;
  }

  [[nodiscard]] const std::string& getName() const noexcept
  {
    return m_name;
  }

protected:
  static int32_t getProperty(const Program& program, uint32_t index, api::ProgramResourceProperty what);

  static std::vector<int32_t>
    getProperty(const Program& program, uint32_t index, api::ProgramResourceProperty what, api::core::SizeType count);

private:
  std::string m_name{};
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

  LocatableProgramInterface(LocatableProgramInterface<_Type>&& rhs) noexcept
      : ProgramInterface<_Type>{std::move(rhs)}
      , m_location{std::exchange(rhs.m_location, -1)}
  {
  }

  LocatableProgramInterface<_Type>& operator=(LocatableProgramInterface<_Type>&& rhs) noexcept
  {
    m_location = std::exchange(rhs.m_location, -1);
    ProgramInterface<_Type>::operator=(std::move(rhs));
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
template<api::ProgramInterface _Type, api::BufferTarget _Target>
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

  ProgramBlock(ProgramBlock<_Type, _Target>&& rhs) noexcept
      : ProgramInterface<_Type>{std::move(rhs)}
      , m_binding{std::exchange(rhs.m_binding, -1)}
  {
  }

  ProgramBlock<_Type, _Target>& operator=(ProgramBlock<_Type, _Target>&& rhs) noexcept
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

  [[nodiscard]] auto getBinding() const noexcept
  {
    return m_binding;
  }

private:
  int32_t m_binding;
};

using ShaderStorageBlock
  = ProgramBlock<api::ProgramInterface::ShaderStorageBlock, api::BufferTarget::ShaderStorageBuffer>;
using UniformBlock = ProgramBlock<api::ProgramInterface::UniformBlock, api::BufferTarget::UniformBuffer>;

class Uniform final : public LocatableProgramInterface<api::ProgramInterface::Uniform>
{
public:
  explicit Uniform(const Program& program, uint32_t index);

  Uniform(Uniform&& rhs) noexcept
      : LocatableProgramInterface{std::move(rhs)}
      , m_size{std::exchange(rhs.m_size, -1)}
      , m_program{std::exchange(rhs.m_program, InvalidProgram)}
  {
  }

  Uniform& operator=(Uniform&& rhs) noexcept
  {
    m_size = std::exchange(rhs.m_size, -1);
    m_program = std::exchange(rhs.m_program, InvalidProgram);
    LocatableProgramInterface::operator=(std::move(rhs));
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

  // NOLINTNEXTLINE(bugprone-reserved-identifier)
  template<typename _Texture>
  void set(const std::shared_ptr<TextureHandle<_Texture>>& textureHandle)
  {
    Expects(m_program != InvalidProgram);
    GL_ASSERT(api::programUniformHandle(m_program, getLocation(), textureHandle->getHandle()));
  }

  template<typename _It> // NOLINT(bugprone-reserved-identifier)
  void setTextures(const _It& begin, const _It& end)
  {
    Expects(m_program != InvalidProgram && m_size >= 0);

    std::vector<uint64_t> handles;
    for(auto it = begin; it != end; ++it)
    {
      handles.emplace_back((*it)->getHandle());
    }

    Expects(handles.size() == static_cast<size_t>(m_size));
    GL_ASSERT(api::programUniformHandle(
      m_program, getLocation(), gsl::narrow_cast<api::core::SizeType>(handles.size()), handles.data()));
  }

  // NOLINTNEXTLINE(bugprone-reserved-identifier)
  template<typename _Texture>
  void set(const std::vector<std::shared_ptr<TextureHandle<_Texture>>>& textureHandles)
  {
    setTextures(textureHandles.begin(), textureHandles.end());
  }

  // NOLINTNEXTLINE(bugprone-reserved-identifier)
  template<typename _Texture, size_t N>
  void set(const std::array<std::shared_ptr<TextureHandle<_Texture>>, N>& textureHandles)
  {
    setTextures(textureHandles.begin(), textureHandles.end());
  }

private:
  static constexpr uint32_t InvalidProgram = std::numeric_limits<uint32_t>::max();

  int32_t m_size = -1;
  uint32_t m_program;
};

class Program final : public BindableResource
{
public:
  explicit Program(const std::string& label = {})
      : BindableResource{[]([[maybe_unused]] const api::core::SizeType n, uint32_t* handle)
                         {
                           BOOST_ASSERT(n == 1 && handle != nullptr);
                           *handle = api::createProgram();
                         },
                         api::useProgram,
                         []([[maybe_unused]] const api::core::SizeType n, const uint32_t* handle)
                         {
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
    GL_ASSERT(api::getProgram(getHandle(), api::ProgramProperty::LinkStatus, &success));
    return success == static_cast<int32_t>(api::Boolean::True);
  }

  [[nodiscard]] std::string getInfoLog() const
  {
    int32_t length = 0;
    GL_ASSERT(api::getProgram(getHandle(), api::ProgramProperty::InfoLogLength, &length));
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

  [[nodiscard]] std::vector<Uniform> getUniforms() const
  {
    return getInputs<Uniform>();
  }

  [[nodiscard]] std::vector<ShaderStorageBlock> getShaderStorageBlocks() const
  {
    return getInputs<ShaderStorageBlock>();
  }

  [[nodiscard]] std::vector<UniformBlock> getUniformBlocks() const
  {
    return getInputs<UniformBlock>();
  }

private:
  template<typename T>
  [[nodiscard]] std::vector<T> getInputs() const
  {
    const auto n = getActiveResourceCount(T::Type);
    std::vector<T> inputs;
    inputs.reserve(n);
    for(uint32_t i = 0; i < n; ++i)
    {
      inputs.emplace_back(*this, i);
    }
    return inputs;
  }
};

// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<api::ProgramInterface _Type>
ProgramInterface<_Type>::ProgramInterface(const Program& program, const uint32_t index)
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

inline Uniform::Uniform(const Program& program, const uint32_t index)
    : LocatableProgramInterface{program, index}
    , m_program{program.getHandle()}
{
  int32_t type;
  GL_ASSERT(api::getActiveUniforms(program.getHandle(), 1, &index, api::UniformPName::UniformType, &type));
  GL_ASSERT(api::getActiveUniforms(program.getHandle(), 1, &index, api::UniformPName::UniformSize, &m_size));
  Expects(m_size >= 0);
}
} // namespace gl
