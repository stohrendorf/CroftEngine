#include "shader.h"

#include "api/gl.hpp"
#include "glassert.h"

#include <array>
#include <boost/algorithm/string/join.hpp>
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <gsl/gsl-lite.hpp>
#include <ios>
#include <iterator>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace gl
{
namespace
{
std::string readAll(const std::filesystem::path& filePath)
{
  if(!std::filesystem::is_regular_file(filePath))
  {
    BOOST_LOG_TRIVIAL(fatal) << "Could not find required file " << filePath;
    BOOST_THROW_EXCEPTION(std::runtime_error("required file not found"));
  }

  std::ifstream stream{filePath, std::ios::in};
  std::noskipws(stream);
  return std::string{std::istream_iterator<char>{stream}, std::istream_iterator<char>{}};
}

std::string replaceDefines(const std::vector<std::string>& defines, bool isInput)
{
  std::string out;
  if(!defines.empty())
  {
    out += std::string("#define ") + boost::algorithm::join(defines, "\n#define ");
  }

  out += isInput ? "\n#define IN_OUT in\n" : "\n#define IN_OUT out\n";

  return out;
}

// NOLINTNEXTLINE(misc-no-recursion)
void replaceIncludes(const std::filesystem::path& filepath,
                     const std::string& source,
                     std::string& out,
                     std::set<std::filesystem::path>& included)
{
  included.emplace(filepath);

  // Replace the #include "foo.bar" with the sourced file contents of "filepath/foo.bar"
  size_t headPos = 0;
  while(headPos < source.length())
  {
    const auto lastPos = headPos;
    if(headPos == 0)
    {
      // find the first "#include"
      headPos = source.find("#include");
    }
    else
    {
      // find the next "#include"
      headPos = source.find("#include", headPos + 1);
    }

    // If "#include" is found
    if(headPos != std::string::npos)
    {
      // append from our last position for the length (head - last position)
      {
        const auto part = source.substr(lastPos, headPos - lastPos);
        out.append(part);
      }

      // find the start quote "
      const size_t startQuote = source.find('"', headPos) + 1;
      if(startQuote == std::string::npos)
      {
        // We have started an "#include" but missing the leading quote "
        BOOST_LOG_TRIVIAL(error) << "Compile failed for shader '" << filepath << "' missing leading \".";
        return;
      }
      // find the end quote "
      const size_t endQuote = source.find('"', startQuote);
      if(endQuote == std::string::npos)
      {
        // We have a start quote but missing the trailing quote "
        BOOST_LOG_TRIVIAL(error) << "Compile failed for shader '" << filepath << "' missing trailing \".";
        return;
      }

      // jump the head position past the end quote
      headPos = endQuote + 1;

      // File path to include and 'stitch' in the value in the quotes to the file path and source it.
      const size_t len = endQuote - startQuote;
      const auto includePath = filepath.parent_path() / std::filesystem::path{source.substr(startQuote, len)};
      if(included.count(includePath) > 0)
      {
        continue;
      }

      const std::string includedSource = readAll(includePath);
      if(includedSource.empty())
      {
        BOOST_LOG_TRIVIAL(error) << "Compile failed for shader '" << filepath << "': failed to include'" << includePath
                                 << "'";
        return;
      }
      // Valid file so lets attempt to see if we need to append anything to it too (recurse...)
      replaceIncludes(includePath, includedSource, out, included);
    }
    else
    {
      // Append the remaining
      out.append(source, lastPos);
      break;
    }
  }
}
} // namespace

// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<api::ShaderType _Type>
Shader<_Type>::Shader(const gsl::span<gsl::czstring>& src, const std::string_view& label)
    : m_handle{GL_ASSERT_FN(api::createShader(Type))}
{
  gsl_Ensures(m_handle != 0);
  GL_ASSERT(api::shaderSource(m_handle, gsl::narrow<api::core::SizeType>(src.size()), src.data(), nullptr));
  GL_ASSERT(api::compileShader(m_handle));

  if(const auto log = getInfoLog(); !log.empty())
    BOOST_LOG_TRIVIAL(debug) << "Shader info log: " << log;

  auto success = static_cast<int>(api::Boolean::False);
  GL_ASSERT(api::getShader(m_handle, api::ShaderParameterName::CompileStatus, &success));
  if(success != static_cast<int>(api::Boolean::True))
  {
    BOOST_LOG_TRIVIAL(error) << "Failed to compile shader program " << label;
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to compile shader program"));
  }

  if(!label.empty())
  {
    GL_ASSERT(api::objectLabel(
      api::ObjectIdentifier::Shader, m_handle, gsl::narrow<api::core::SizeType>(label.size()), label.data()));
  }
}

// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<api::ShaderType _Type>
std::string Shader<_Type>::getInfoLog() const
{
  int32_t length = 0;
  GL_ASSERT(api::getShader(m_handle, api::ShaderParameterName::InfoLogLength, &length));
  if(length == 0)
  {
    length = 4096;
  }
  if(length <= 0)
    return {};

  std::vector<char> infoLog(length, '\0');
  GL_ASSERT(api::getShaderInfoLog(m_handle, length, nullptr, infoLog.data()));
  infoLog.back() = '\0';
  return infoLog.data();
}

// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<api::ShaderType _Type>
Shader<_Type> Shader<_Type>::create(const std::filesystem::path& sourcePath,
                                    const std::string& source,
                                    const std::vector<std::string>& defines,
                                    const std::string_view& label)
{
  static constexpr size_t SHADER_SOURCE_LENGTH = 3;
  std::array<gsl::czstring, SHADER_SOURCE_LENGTH> shaderSource{nullptr};
  shaderSource[0]
    = "#version 450 core\n"
      "#extension GL_ARB_bindless_texture : require\n"
      "#extension GL_ARB_gpu_shader5 : require\n";

  const std::string definesStr = replaceDefines(defines, _Type == api::ShaderType::FragmentShader);
  shaderSource[1] = definesStr.c_str();

  std::string sourceStr;
  if(!sourcePath.empty())
  {
    // Replace the #include "foo.bar" with the sources that come from file paths
    std::set<std::filesystem::path> included;
    replaceIncludes(sourcePath, source, sourceStr, included);
  }
  shaderSource[2] = !sourcePath.empty() ? sourceStr.c_str() : source.c_str();

  return Shader<_Type>{shaderSource, label};
}

// NOLINTNEXTLINE(bugprone-reserved-identifier)
template<api::ShaderType _Type>
Shader<_Type> Shader<_Type>::create(const std::filesystem::path& sourcePath,
                                    const std::vector<std::string>& defines,
                                    const std::string_view& label)
{
  if(!std::filesystem::is_regular_file(sourcePath))
  {
    BOOST_LOG_TRIVIAL(fatal) << "Could not find required file " << sourcePath;
    BOOST_THROW_EXCEPTION(std::runtime_error("required file not found"));
  }

  const std::string source = readAll(sourcePath);
  if(source.empty())
  {
    BOOST_LOG_TRIVIAL(error) << "Failed to read vertex shader from file '" << sourcePath << "'.";
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create shader from sources"));
  }

  return create(sourcePath, source, defines, label);
}

template class Shader<api::ShaderType::FragmentShader>;
template class Shader<api::ShaderType::ComputeShader>;
template class Shader<api::ShaderType::GeometryShader>;
template class Shader<api::ShaderType::TessControlShader>;
template class Shader<api::ShaderType::TessEvaluationShader>;
template class Shader<api::ShaderType::VertexShader>;
} // namespace gl
