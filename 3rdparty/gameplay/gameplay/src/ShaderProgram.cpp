#include "Base.h"
#include "ShaderProgram.h"
#include "Game.h"

#include "gl/shader.h"

#include <fstream>

#include <boost/log/trivial.hpp>
#include <boost/algorithm/string.hpp>

namespace gameplay
{
namespace
{
std::string readAll(const std::string& filePath)
{
    // Open file for reading.
    std::ifstream stream( filePath, std::ios::in | std::ios::binary );
    if( !stream.is_open() )
    {
        BOOST_LOG_TRIVIAL( error ) << "Failed to load file: " << filePath;
        return {};
    }
    stream.seekg( 0, std::ios::end );
    auto size = static_cast<std::size_t>(stream.tellg());
    stream.seekg( 0, std::ios::beg );

    // Read entire file contents.
    std::string buffer;
    buffer.resize( size );
    stream.read( &buffer[0], size );
    if( static_cast<std::size_t>(stream.gcount()) != size )
    {
        BOOST_LOG_TRIVIAL( error ) << "Failed to read complete contents of file '" << filePath
                                   << "' (amount read vs. file size: " << stream.gcount() << " < " << size
                                   << ").";
        return {};
    }

    return buffer;
}

std::string replaceDefines(const std::vector<std::string>& defines)
{
    std::string out;
    if( !defines.empty() )
    {
        out += std::string( "\n#define " ) + boost::algorithm::join( defines, "\n#define " );
    }

    if( !out.empty() )
    {
        out += "\n";
    }

    return out;
}

void replaceIncludes(const std::string& filepath, const std::string& source, std::string& out)
{
    // Replace the #include "xxxx.xxx" with the sourced file contents of "filepath/xxxx.xxx"
    std::string str = source;
    size_t headPos = 0;
    const auto fileLen = str.length();
    const auto tailPos = fileLen;
    while( headPos < fileLen )
    {
        const auto lastPos = headPos;
        if( headPos == 0 )
        {
            // find the first "#include"
            headPos = str.find( "#include" );
        }
        else
        {
            // find the next "#include"
            headPos = str.find( "#include", headPos + 1 );
        }

        // If "#include" is found
        if( headPos != std::string::npos )
        {
            // append from our last position for the legth (head - last position)
            out.append( str.substr( lastPos, headPos - lastPos ) );

            // find the start quote "
            size_t startQuote = str.find( "\"", headPos ) + 1;
            if( startQuote == std::string::npos )
            {
                // We have started an "#include" but missing the leading quote "
                BOOST_LOG_TRIVIAL( error ) << "Compile failed for shader '" << filepath
                                           << "' missing leading \".";
                return;
            }
            // find the end quote "
            size_t endQuote = str.find( "\"", startQuote );
            if( endQuote == std::string::npos )
            {
                // We have a start quote but missing the trailing quote "
                BOOST_LOG_TRIVIAL( error ) << "Compile failed for shader '" << filepath
                                           << "' missing trailing \".";
                return;
            }

            // jump the head position past the end quote
            headPos = endQuote + 1;

            // File path to include and 'stitch' in the value in the quotes to the file path and source it.
            std::string filepathStr = filepath;
            std::string directoryPath = filepathStr.substr( 0, filepathStr.rfind( '/' ) + 1 );
            size_t len = endQuote - startQuote;
            std::string includeStr = str.substr( startQuote, len );
            directoryPath.append( includeStr );
            std::string includedSource = readAll( directoryPath );
            if( includedSource.empty() )
            {
                BOOST_LOG_TRIVIAL( error ) << "Compile failed for shader '" << filepathStr
                                           << "' invalid filepath.";
                return;
            }
            // Valid file so lets attempt to see if we need to append anything to it too (recurse...)
            replaceIncludes( directoryPath, includedSource, out );
        }
        else
        {
            // Append the remaining
            out.append( str, lastPos, tailPos );
        }
    }
}

void writeShaderToErrorFile(const std::string& filePath, const std::string& source)
{
    std::ofstream stream{filePath + ".err", std::ios::out | std::ios::binary | std::ios::trunc};
    stream.write( source.c_str(), source.size() );
}
}

ShaderProgram::ShaderProgram() = default;

ShaderProgram::~ShaderProgram()
{
    m_uniforms.clear();
}

std::shared_ptr<ShaderProgram> ShaderProgram::createFromFile(const std::string& vshPath,
                                                             const std::string& fshPath,
                                                             const std::vector<std::string>& defines)
{
    // Search the effect cache for an identical effect that is already loaded.
    std::string uniqueId = vshPath;
    uniqueId += ';';
    uniqueId += fshPath;
    uniqueId += ';';
    uniqueId += boost::algorithm::join( defines, ";" );

    // Read source from file.
    std::string vshSource = readAll( vshPath );
    if( vshSource.empty() )
    {
        BOOST_LOG_TRIVIAL( error ) << "Failed to read vertex shader from file '" << vshPath << "'.";
        return nullptr;
    }
    std::string fshSource = readAll( fshPath );
    if( fshSource.empty() )
    {
        BOOST_LOG_TRIVIAL( error ) << "Failed to read fragment shader from file '" << fshPath << "'.";
        return nullptr;
    }

    std::shared_ptr<ShaderProgram> shaderProgram = createFromSource( vshPath, vshSource, fshPath, fshSource,
                                                                     defines );

    if( shaderProgram == nullptr )
    {
        BOOST_LOG_TRIVIAL( error ) << "Failed to create effect from shaders '" << vshPath << "', '" << fshPath
                                   << "'.";
    }
    else
    {
        // Store this effect in the cache.
        shaderProgram->m_id = uniqueId;
    }

    return shaderProgram;
}

std::shared_ptr<ShaderProgram>
ShaderProgram::createFromSource(const std::string& vshPath, const std::string& vshSource,
                                const std::string& fshPath, const std::string& fshSource,
                                const std::vector<std::string>& defines)
{
    // Replace all comma separated definitions with #define prefix and \n suffix
    std::string definesStr = replaceDefines( defines );
    definesStr += "\n";

    const size_t SHADER_SOURCE_LENGTH = 3;
    const GLchar* shaderSource[SHADER_SOURCE_LENGTH];
    shaderSource[0] = "#version 410\n";
    shaderSource[1] = definesStr.c_str();

    std::string vshSourceStr;
    if( !vshPath.empty() )
    {
        // Replace the #include "xxxxx.xxx" with the sources that come from file paths
        replaceIncludes( vshPath, vshSource, vshSourceStr );
        if( !vshSource.empty() )
            vshSourceStr += "\n";
    }
    shaderSource[2] = !vshPath.empty() ? vshSourceStr.c_str() : vshSource.c_str();

    gl::Shader vertexShader{GL_VERTEX_SHADER, vshPath + ";" + boost::algorithm::join( defines, ";" )};
    vertexShader.setSource( shaderSource, SHADER_SOURCE_LENGTH );
    vertexShader.compile();
    if( !vertexShader.getCompileStatus() )
    {
        // Write out the expanded shader file.
        if( !vshPath.empty() )
            writeShaderToErrorFile( vshPath, shaderSource[2] );

        BOOST_LOG_TRIVIAL( error ) << "Compile failed for vertex shader '" << (vshPath.empty() ? "<none>" : vshPath)
                                   << "' with error '" << vertexShader.getInfoLog() << "'.";

        return nullptr;
    }

    // Compile the fragment shader.
    std::string fshSourceStr;
    if( !fshPath.empty() )
    {
        // Replace the #include "xxxxx.xxx" with the sources that come from file paths
        replaceIncludes( fshPath, fshSource, fshSourceStr );
        if( !fshSource.empty() )
            fshSourceStr += "\n";
    }
    shaderSource[2] = !fshPath.empty() ? fshSourceStr.c_str() : fshSource.c_str();

    gl::Shader fragmentShader{GL_FRAGMENT_SHADER, fshPath + ";" + boost::algorithm::join( defines, ";" )};
    fragmentShader.setSource( shaderSource, SHADER_SOURCE_LENGTH );
    fragmentShader.compile();
    if( !fragmentShader.getCompileStatus() )
    {
        // Write out the expanded shader file.
        if( !fshPath.empty() )
            writeShaderToErrorFile( fshPath, shaderSource[2] );

        BOOST_LOG_TRIVIAL( error ) << "Compile failed for fragment shader '"
                                   << (fshPath.empty() ? "<none>" : fshPath) << "' with error '"
                                   << fragmentShader.getInfoLog() << "'.";

        return nullptr;
    }

    // Create and return the new Effect.
    auto shaderProgram = std::make_shared<ShaderProgram>();
    shaderProgram->m_handle.attach( vertexShader );
    shaderProgram->m_handle.attach( fragmentShader );
    shaderProgram->m_handle.link( vshPath + ";" + fshPath + ";" + boost::algorithm::join( defines, ";" ) );

    // Check link status.
    if( !shaderProgram->m_handle.getLinkStatus() )
    {
        BOOST_LOG_TRIVIAL( error ) << "Linking program failed (" << (vshPath.empty() ? "<none>" : vshPath) << ","
                                   << (fshPath.empty() ? "<none>" : fshPath) << "): "
                                   << shaderProgram->m_handle.getInfoLog();

        return nullptr;
    }

    // Query and store vertex attribute meta-data from the program.
    // NOTE: Rather than using glBindAttribLocation to explicitly specify our own
    // preferred attribute locations, we're going to query the locations that were
    // automatically bound by the GPU. While it can sometimes be convenient to use
    // glBindAttribLocation, some vendors actually reserve certain attribute indices
    // and therefore using this function can create compatibility issues between
    // different hardware vendors.

    for( auto&& attrib : shaderProgram->m_handle.getActiveAttributes() )
    {
        shaderProgram->m_vertexAttributes.insert( make_pair( attrib.getName(), std::move( attrib ) ) );
    }

    for( auto&& uniform : shaderProgram->m_handle.getActiveUniforms() )
    {
        shaderProgram->m_uniforms.emplace( make_pair( uniform.getName(), std::move( uniform ) ) );
    }

    return shaderProgram;
}

const std::string& ShaderProgram::getId() const
{
    return m_id;
}

const gl::Program::ActiveAttribute* ShaderProgram::getVertexAttribute(const std::string& name) const
{
    auto it = m_vertexAttributes.find( name );
    return it == m_vertexAttributes.end() ? nullptr : &it->second;
}

gl::Program::ActiveUniform* ShaderProgram::getUniform(const std::string& name) const
{
    auto it = m_uniforms.find( name );
    return it == m_uniforms.end() ? nullptr : &it->second;
}

void ShaderProgram::bind()
{
    m_handle.bind();
}
}
