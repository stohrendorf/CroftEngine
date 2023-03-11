#pragma once

#include "buffer.h"
#include "glassert.h"
#include "soglb_fwd.h"

#include <array>
#include <boost/assert.hpp>
#include <cstddef>
#include <cstdint>
#include <glm/fwd.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

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
    gsl_Ensures(m_binding >= 0);
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
    gsl_Expects(m_binding >= 0);
    GL_ASSERT(api::bindBufferBase(_Target, m_binding, buffer.getHandle()));
  }

  template<typename T, api::BufferTarget LazyTarget = _Target>
  auto bindRange(const Buffer<T, _Target>& buffer, size_t start, size_t n) -> std::enable_if_t<
    LazyTarget == api::BufferTarget::AtomicCounterBuffer || LazyTarget == api::BufferTarget::TransformFeedbackBuffer
      || LazyTarget == api::BufferTarget::UniformBuffer || LazyTarget == api::BufferTarget::ShaderStorageBuffer,
    void>
  {
    gsl_Expects(start < buffer.size());
    gsl_Expects(start + n <= buffer.size());
    GL_ASSERT(api::bindBufferRange(_Target, m_binding, buffer.getHandle(), sizeof(T) * start, sizeof(T) * n));
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
  Uniform(Uniform&& rhs) noexcept;
  Uniform& operator=(Uniform&& rhs) noexcept;

  template<typename T>
  std::enable_if_t<std::is_trivial_v<T>, void> set(const T& value)
  {
    gsl_Expects(m_program != InvalidProgram);
    if(changeValue(value))
      GL_ASSERT(api::programUniform1(m_program, getLocation(), value)); // cppcheck-suppress missingReturn
  }

  template<typename T>
  void set(const std::vector<T>& values)
  {
    gsl_Expects(m_program != InvalidProgram);
    if(changeValue(values))
      GL_ASSERT(
        api::programUniform1(m_program, getLocation(), gsl::narrow<api::core::SizeType>(values.size()), values.data()));
  }

  template<typename T, size_t N>
  void set(const std::array<T, N>& values)
  {
    gsl_Expects(m_program != InvalidProgram);
    if(changeValue(std::vector{values.begin(), values.end()}))
      GL_ASSERT(
        api::programUniform1(m_program, getLocation(), gsl::narrow<api::core::SizeType>(values.size()), values.data()));
  }

  void set(const glm::mat3& value);

  void set(const glm::mat4& value);

  void set(const std::vector<glm::mat4>& values);

  void set(const glm::vec2& value);

  void set(const std::vector<glm::vec2>& values);

  void set(const std::vector<glm::vec3>& values);

  void set(const std::vector<glm::vec4>& values);

  void set(const glm::vec3& value);

  void set(const glm::vec4& value);

  // NOLINTNEXTLINE(bugprone-reserved-identifier)
  template<typename _Texture>
  void set(const gslu::nn_shared<TextureHandle<_Texture>>& textureHandle)
  {
    gsl_Expects(m_program != InvalidProgram);
    gsl_Assert(GL_ASSERT_FN(gl::api::isTextureHandleResident(textureHandle->getHandle())));
    if(changeValue(textureHandle->getHandle()))
      GL_ASSERT(api::programUniformHandle(m_program, getLocation(), textureHandle->getHandle()));
  }

  template<typename _It> // NOLINT(bugprone-reserved-identifier)
  void setTextures(const _It& begin, const _It& end)
  {
    gsl_Expects(m_program != InvalidProgram && m_size >= 0);

    std::vector<uint64_t> handles;
    for(auto it = begin; it != end; ++it)
    {
      handles.emplace_back((*it)->getHandle());
      gsl_Assert(GL_ASSERT_FN(gl::api::isTextureHandleResident(handles.back())));
    }

    gsl_Assert(handles.size() == static_cast<size_t>(m_size));
    if(changeValue(handles))
    {
      auto location = getLocation();
      for(const auto& handle : handles)
      {
        GL_ASSERT(api::programUniformHandle(m_program, getLocation(), handle));
        ++location;
      }
    }
  }

  // NOLINTNEXTLINE(bugprone-reserved-identifier)
  template<typename _Texture>
  void set(const gsl::span<const gslu::nn_shared<TextureHandle<_Texture>>>& textureHandles)
  {
    setTextures(textureHandles.begin(), textureHandles.end());
  }

private:
  static constexpr uint32_t InvalidProgram = std::numeric_limits<uint32_t>::max();

  int32_t m_size = -1;
  uint32_t m_program;

  std::variant<std::vector<glm::int32_t>,
               std::vector<glm::float32_t>,
               std::vector<glm::mat3>,
               std::vector<glm::mat4>,
               std::vector<glm::vec2>,
               std::vector<glm::vec3>,
               std::vector<glm::vec4>,
               std::vector<glm::uint64_t>>
    m_value;

  template<typename T>
  bool changeValue(const T& value)
  {
    return changeValue(std::vector{value});
  }

  template<typename T>
  bool changeValue(const std::vector<T>& values)
  {
    if(std::holds_alternative<std::vector<T>>(m_value) && std::get<std::vector<T>>(m_value) == values)
      return false;

    m_value = values;
    return true;
  }
};

class Program final : public BindableResource<api::ObjectIdentifier::Program>
{
public:
  // NOLINTNEXTLINE(bugprone-reserved-identifier)
  template<api::ShaderType... _Types>
  explicit Program(const std::string_view& label, const Shader<_Types>&... shaders)
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
                         label}
  {
    (...,
     [this, &shaders]()
     {
       GL_ASSERT(api::attachShader(getHandle(), shaders.getHandle()));
     }());
    GL_ASSERT(api::linkProgram(getHandle()));
  }

  [[nodiscard]] bool getLinkStatus() const;

  [[nodiscard]] std::string getInfoLog() const;

  [[nodiscard]] uint32_t getActiveResourceCount(api::ProgramInterface what) const;

  [[nodiscard]] std::vector<ProgramInput> getInputs() const;

  [[nodiscard]] std::vector<ProgramOutput> getOutputs() const;

  [[nodiscard]] std::vector<Uniform> getUniforms() const;

  [[nodiscard]] std::vector<ShaderStorageBlock> getShaderStorageBlocks() const;

  [[nodiscard]] std::vector<UniformBlock> getUniformBlocks() const;

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
  gsl_Assert(nameLength > 0);
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
} // namespace gl
