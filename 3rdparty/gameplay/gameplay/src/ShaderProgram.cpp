#include "Base.h"
#include "ShaderProgram.h"
#include "FileSystem.h"
#include "Game.h"

#include "gl/shader.h"

#include <glm/gtc/type_ptr.hpp>

#include <boost/log/trivial.hpp>
#include <boost/algorithm/string.hpp>


namespace gameplay
{
    // Cache of unique effects.
    static ShaderProgram* __currentEffect = nullptr;


    ShaderProgram::ShaderProgram() = default;


    ShaderProgram::~ShaderProgram()
    {
        // Free uniforms.
        _uniforms.clear();

        // If our program object is currently bound, unbind it before we're destroyed.
        if( __currentEffect == this )
        {
            m_handle.unbind();
            __currentEffect = nullptr;
        }
    }


    std::shared_ptr<ShaderProgram> ShaderProgram::createFromFile(const std::string& vshPath, const std::string& fshPath, const std::vector<std::string>& defines)
    {
        // Search the effect cache for an identical effect that is already loaded.
        std::string uniqueId = vshPath;
        uniqueId += ';';
        uniqueId += fshPath;
        uniqueId += ';';
        uniqueId += boost::algorithm::join(defines, ";");

        // Read source from file.
        std::string vshSource = FileSystem::readAll(vshPath);
        if( vshSource.empty() )
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to read vertex shader from file '" << vshPath << "'.";
            return nullptr;
        }
        std::string fshSource = FileSystem::readAll(fshPath);
        if( fshSource.empty() )
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to read fragment shader from file '" << fshPath << "'.";
            return nullptr;
        }

        std::shared_ptr<ShaderProgram> shaderProgram = createFromSource(vshPath, vshSource, fshPath, fshSource, defines);

        if( shaderProgram == nullptr )
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to create effect from shaders '" << vshPath << "', '" << fshPath << "'.";
        }
        else
        {
            // Store this effect in the cache.
            shaderProgram->_id = uniqueId;
        }

        return shaderProgram;
    }


    std::shared_ptr<ShaderProgram> ShaderProgram::createFromSource(const std::string& vshSource, const std::string& fshSource, const std::vector<std::string>& defines)
    {
        return createFromSource("", vshSource, "", fshSource, defines);
    }


    static std::string replaceDefines(const std::vector<std::string>& defines)
    {
        std::string out;
        if( !defines.empty() )
        {
            out += std::string("\n#define ") + boost::algorithm::join(defines, "\n#define ");
        }

        if( !out.empty() )
        {
            out += "\n";
        }

        return out;
    }


    static void replaceIncludes(const std::string& filepath, const std::string& source, std::string& out)
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
                headPos = str.find("#include");
            }
            else
            {
                // find the next "#include"
                headPos = str.find("#include", headPos + 1);
            }

            // If "#include" is found
            if( headPos != std::string::npos )
            {
                // append from our last position for the legth (head - last position)
                out.append(str.substr(lastPos, headPos - lastPos));

                // find the start quote "
                size_t startQuote = str.find("\"", headPos) + 1;
                if( startQuote == std::string::npos )
                {
                    // We have started an "#include" but missing the leading quote "
                    BOOST_LOG_TRIVIAL(error) << "Compile failed for shader '" << filepath << "' missing leading \".";
                    return;
                }
                // find the end quote "
                size_t endQuote = str.find("\"", startQuote);
                if( endQuote == std::string::npos )
                {
                    // We have a start quote but missing the trailing quote "
                    BOOST_LOG_TRIVIAL(error) << "Compile failed for shader '" << filepath << "' missing trailing \".";
                    return;
                }

                // jump the head position past the end quote
                headPos = endQuote + 1;

                // File path to include and 'stitch' in the value in the quotes to the file path and source it.
                std::string filepathStr = filepath;
                std::string directoryPath = filepathStr.substr(0, filepathStr.rfind('/') + 1);
                size_t len = endQuote - (startQuote);
                std::string includeStr = str.substr(startQuote, len);
                directoryPath.append(includeStr);
                std::string includedSource = FileSystem::readAll(directoryPath.c_str());
                if( includedSource.empty() )
                {
                    BOOST_LOG_TRIVIAL(error) << "Compile failed for shader '" << filepathStr << "' invalid filepath.";
                    return;
                }
                else
                {
                    // Valid file so lets attempt to see if we need to append anything to it too (recurse...)
                    replaceIncludes(directoryPath, includedSource, out);
                }
            }
            else
            {
                // Append the remaining
                out.append(str, lastPos, tailPos);
            }
        }
    }


    static void writeShaderToErrorFile(const std::string& filePath, const std::string& source)
    {
        std::string path = filePath + ".err";
        std::unique_ptr<Stream> stream(FileSystem::open(path.c_str(), FileSystem::WRITE));
        if( stream.get() != nullptr && stream->canWrite() )
        {
            stream->write(source.c_str(), 1, source.size());
        }
    }


    std::shared_ptr<ShaderProgram> ShaderProgram::createFromSource(const std::string& vshPath, const std::string& vshSource, const std::string& fshPath, const std::string& fshSource, const std::vector<std::string>& defines)
    {
        // Replace all comma separated definitions with #define prefix and \n suffix
        std::string definesStr = replaceDefines(defines);
        definesStr += "\n";

        const size_t SHADER_SOURCE_LENGTH = 3;
        const GLchar* shaderSource[SHADER_SOURCE_LENGTH];
        shaderSource[0] = "#version 410\n";
        shaderSource[1] = definesStr.c_str();

        std::string vshSourceStr;
        if( !vshPath.empty() )
        {
            // Replace the #include "xxxxx.xxx" with the sources that come from file paths
            replaceIncludes(vshPath, vshSource, vshSourceStr);
            if( !vshSource.empty() )
                vshSourceStr += "\n";
        }
        shaderSource[2] = !vshPath.empty() ? vshSourceStr.c_str() : vshSource.c_str();

        gl::Shader vertexShader{GL_VERTEX_SHADER};
        vertexShader.setSource(shaderSource, SHADER_SOURCE_LENGTH);
        vertexShader.compile();
        if( !vertexShader.getCompileStatus() )
        {
            // Write out the expanded shader file.
            if( !vshPath.empty() )
                writeShaderToErrorFile(vshPath, shaderSource[2]);

            BOOST_LOG_TRIVIAL(error) << "Compile failed for vertex shader '" << (vshPath.empty() ? "<none>" : vshPath) << "' with error '" << vertexShader.getInfoLog() << "'.";

            return nullptr;
        }

        // Compile the fragment shader.
        std::string fshSourceStr;
        if( !fshPath.empty() )
        {
            // Replace the #include "xxxxx.xxx" with the sources that come from file paths
            replaceIncludes(fshPath, fshSource, fshSourceStr);
            if( !fshSource.empty() )
                fshSourceStr += "\n";
        }
        shaderSource[2] = !fshPath.empty() ? fshSourceStr.c_str() : fshSource.c_str();

        gl::Shader fragmentShader{GL_FRAGMENT_SHADER};
        fragmentShader.setSource(shaderSource, SHADER_SOURCE_LENGTH);
        fragmentShader.compile();
        if( !fragmentShader.getCompileStatus() )
        {
            // Write out the expanded shader file.
            if( !fshPath.empty() )
                writeShaderToErrorFile(fshPath, shaderSource[2]);

            BOOST_LOG_TRIVIAL(error) << "Compile failed for fragment shader '" << (fshPath.empty() ? "<none>" : fshPath) << "' with error '" << fragmentShader.getInfoLog() << "'.";

            return nullptr;
        }


        // Create and return the new Effect.
        auto shaderProgram = std::make_shared<ShaderProgram>();
        shaderProgram->m_handle.attach(vertexShader);
        shaderProgram->m_handle.attach(fragmentShader);
        shaderProgram->m_handle.link();

        // Check link status.
        if( !shaderProgram->m_handle.getLinkStatus() )
        {
            BOOST_LOG_TRIVIAL(error) << "Linking program failed (" << (vshPath.empty() ? "<none>" : vshPath) << "," << (fshPath.empty() ? "<none>" : fshPath) << "): " << shaderProgram->m_handle.getInfoLog();

            return nullptr;
        }

        // Query and store vertex attribute meta-data from the program.
        // NOTE: Rather than using glBindAttribLocation to explicitly specify our own
        // preferred attribute locations, we're going to query the locations that were
        // automatically bound by the GPU. While it can sometimes be convenient to use
        // glBindAttribLocation, some vendors actually reserve certain attribute indices
        // and therefore using this function can create compatibility issues between
        // different hardware vendors.

        for(auto&& attrib : shaderProgram->m_handle.getActiveAttributes())
        {
            shaderProgram->_vertexAttributes.insert(std::make_pair(attrib.getName(), std::move(attrib)));
        }

        for(auto&& uniform : shaderProgram->m_handle.getActiveUniforms())
        {
            shaderProgram->_uniforms.insert(std::make_pair(uniform.getName(), std::move(uniform)));
        }

        return shaderProgram;
    }


    const std::string& ShaderProgram::getId() const
    {
        return _id;
    }


    const gl::Program::ActiveAttribute* ShaderProgram::getVertexAttribute(const std::string& name) const
    {
        auto itr = _vertexAttributes.find(name);
        return itr == _vertexAttributes.end() ? nullptr : &itr->second;
    }


    gl::Program::ActiveUniform* ShaderProgram::getUniform(const std::string& name) const
    {
        auto itr = _uniforms.find(name);
        return itr == _uniforms.end() ? nullptr : &itr->second;
    }


    gl::Program::ActiveUniform* ShaderProgram::getUniform(size_t index) const
    {
        size_t i = 0;
        for( auto itr = _uniforms.begin(); itr != _uniforms.end(); ++itr , ++i )
        {
            if( i == index )
            {
                return &itr->second;
            }
        }
        return nullptr;
    }


    size_t ShaderProgram::getUniformCount() const
    {
        return _uniforms.size();
    }


    void ShaderProgram::bind()
    {
        m_handle.bind();
        __currentEffect = this;
    }
}
