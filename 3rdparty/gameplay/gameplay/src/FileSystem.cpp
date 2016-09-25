#include "Base.h"
#include "FileSystem.h"
#include "Properties.h"
#include "Stream.h"

#include <boost/log/trivial.hpp>

#ifdef WIN32
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#define gp_stat _stat
#define gp_stat_struct struct stat
#else
    #define __EXT_POSIX2
    #include <libgen.h>
    #include <dirent.h>
    #define gp_stat stat
    #define gp_stat_struct struct stat
#endif


namespace gameplay
{
    /** @script{ignore} */
    static std::string __resourcePath("./");
    static std::string __assetPath("");
    static std::map<std::string, std::string> __aliases;


    /**
     * Gets the fully resolved path.
     * If the path is relative then it will be prefixed with the resource path.
     * Aliases will be converted to a relative path.
     *
     * @param path The path to resolve.
     * @param fullPath The full resolved path. (out param)
     */
    static void getFullPath(const char* path, std::string& fullPath)
    {
        if( FileSystem::isAbsolutePath(path) )
        {
            fullPath.assign(path);
        }
        else
        {
            fullPath.assign(__resourcePath);
            fullPath += FileSystem::resolvePath(path);
        }
    }


    /**
     *
     * @script{ignore}
     */
    class FileStream : public Stream
    {
    public:
        friend class FileSystem;

        ~FileStream();
        virtual bool canRead();
        virtual bool canWrite();
        virtual bool canSeek();
        virtual void close();
        virtual size_t read(void* ptr, size_t size, size_t count);
        virtual char* readLine(char* str, int num);
        virtual size_t write(const void* ptr, size_t size, size_t count);
        virtual bool eof();
        virtual size_t length();
        virtual long int position();
        virtual bool seek(long int offset, int origin);
        virtual bool rewind();

        static FileStream* create(const char* filePath, const char* mode);

    private:
        FileStream(FILE* file);

    private:
        FILE* _file;
        bool _canRead;
        bool _canWrite;
    };

    /////////////////////////////

    FileSystem::FileSystem()
    {
    }


    FileSystem::~FileSystem()
    {
    }


    void FileSystem::setResourcePath(const char* path)
    {
        __resourcePath = path == nullptr ? "" : path;
    }


    const char* FileSystem::getResourcePath()
    {
        return __resourcePath.c_str();
    }


    void FileSystem::loadResourceAliases(const char* aliasFilePath)
    {
        Properties* properties = Properties::create(aliasFilePath);
        if( properties )
        {
            Properties* aliases;
            while( (aliases = properties->getNextNamespace()) != nullptr )
            {
                loadResourceAliases(aliases);
            }
        }
        SAFE_DELETE(properties);
    }


    void FileSystem::loadResourceAliases(Properties* properties)
    {
        assert(properties);

        const char* name;
        while( (name = properties->getNextProperty()) != nullptr )
        {
            __aliases[name] = properties->getString();
        }
    }


    const char* FileSystem::resolvePath(const char* path)
    {
        BOOST_ASSERT(path);

        size_t len = strlen(path);
        if( len > 1 && path[0] == '@' )
        {
            std::string alias(path + 1);
            std::map<std::string, std::string>::const_iterator itr = __aliases.find(alias);
            if( itr == __aliases.end() )
                return path; // no matching alias found
            return itr->second.c_str();
        }

        return path;
    }


    bool FileSystem::fileExists(const char* filePath)
    {
        BOOST_ASSERT(filePath);

        std::string fullPath;

        getFullPath(filePath, fullPath);

        gp_stat_struct s;
        return stat(fullPath.c_str(), &s) == 0;
    }


    Stream* FileSystem::open(const char* path, size_t streamMode)
    {
        char modeStr[] = "rb";
        if( (streamMode & WRITE) != 0 )
            modeStr[0] = 'w';

        std::string fullPath;
        getFullPath(path, fullPath);
        FileStream* stream = FileStream::create(fullPath.c_str(), modeStr);
        return stream;
    }


    FILE* FileSystem::openFile(const char* filePath, const char* mode)
    {
        BOOST_ASSERT(filePath);
        BOOST_ASSERT(mode);

        std::string fullPath;
        getFullPath(filePath, fullPath);

        FILE* fp = fopen(fullPath.c_str(), mode);
        return fp;
    }


    std::string FileSystem::readAll(const std::string& filePath, size_t* fileSize)
    {
        // Open file for reading.
        std::unique_ptr<Stream> stream(open(filePath.c_str()));
        if( stream.get() == nullptr )
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to load file: " << filePath;
            return {};
        }
        size_t size = stream->length();

        // Read entire file contents.
        std::string buffer;
        buffer.resize(size);
        size_t read = stream->read(&buffer[0], 1, size);
        if( read != size )
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to read complete contents of file '" << filePath << "' (amount read vs. file size: " << read << " < " << size << ").";
            return {};
        }

        if( fileSize )
        {
            *fileSize = size;
        }
        return buffer;
    }


    bool FileSystem::isAbsolutePath(const char* filePath)
    {
        if( filePath == 0 || filePath[0] == '\0' )
            return false;
#ifdef WIN32
        if( filePath[1] != '\0' )
        {
            char first = filePath[0];
            return (filePath[1] == ':' && ((first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z')));
        }
        return false;
#else
        return filePath[0] == '/';
#endif
    }


    void FileSystem::setAssetPath(const char* path)
    {
        __assetPath = path;
    }


    const char* FileSystem::getAssetPath()
    {
        return __assetPath.c_str();
    }


    std::string FileSystem::getDirectoryName(const char* path)
    {
        if( path == nullptr || strlen(path) == 0 )
        {
            return "";
        }
#ifdef WIN32
        char drive[_MAX_DRIVE ];
        char dir[_MAX_DIR ];
        _splitpath(path, drive, dir, nullptr, nullptr);
        std::string dirname;
        size_t driveLength = strlen(drive);
        if( driveLength > 0 )
        {
            dirname.reserve(driveLength + strlen(dir));
            dirname.append(drive);
            dirname.append(dir);
        }
        else
        {
            dirname.assign(dir);
        }
        std::replace(dirname.begin(), dirname.end(), '\\', '/');
        return dirname;
#else
        // dirname() modifies the input string so create a temp string
    std::string dirname;
    char* tempPath = new char[strlen(path) + 1];
    strcpy(tempPath, path);
    char* dir = ::dirname(tempPath);
    if (dir && strlen(dir) > 0)
    {
        dirname.assign(dir);
        // dirname() strips off the trailing '/' so add it back to be consistent with Windows
        dirname.append("/");
    }
    SAFE_DELETE_ARRAY(tempPath);
    return dirname;
#endif
    }


    std::string FileSystem::getExtension(const char* path)
    {
        const char* str = strrchr(path, '.');
        if( str == nullptr )
            return "";

        std::string ext;
        size_t len = strlen(str);
        for( size_t i = 0; i < len; ++i )
            ext += std::toupper(str[i]);

        return ext;
    }


    //////////////////

    FileStream::FileStream(FILE* file)
        : _file(file)
        , _canRead(false)
        , _canWrite(false)
    {
    }


    FileStream::~FileStream()
    {
        if( _file )
        {
            close();
        }
    }


    FileStream* FileStream::create(const char* filePath, const char* mode)
    {
        FILE* file = fopen(filePath, mode);
        if( file )
        {
            FileStream* stream = new FileStream(file);
            const char* s = mode;
            while( s != nullptr && *s != '\0' )
            {
                if( *s == 'r' )
                    stream->_canRead = true;
                else if( *s == 'w' )
                    stream->_canWrite = true;
                ++s;
            }

            return stream;
        }
        return nullptr;
    }


    bool FileStream::canRead()
    {
        return _file && _canRead;
    }


    bool FileStream::canWrite()
    {
        return _file && _canWrite;
    }


    bool FileStream::canSeek()
    {
        return _file != nullptr;
    }


    void FileStream::close()
    {
        if( _file )
            fclose(_file);
        _file = nullptr;
    }


    size_t FileStream::read(void* ptr, size_t size, size_t count)
    {
        if( !_file )
            return 0;
        return fread(ptr, size, count, _file);
    }


    char* FileStream::readLine(char* str, int num)
    {
        if( !_file )
            return 0;
        return fgets(str, num, _file);
    }


    size_t FileStream::write(const void* ptr, size_t size, size_t count)
    {
        if( !_file )
            return 0;
        return fwrite(ptr, size, count, _file);
    }


    bool FileStream::eof()
    {
        if( !_file || feof(_file) )
            return true;
        return ((size_t)position()) >= length();
    }


    size_t FileStream::length()
    {
        size_t len = 0;
        if( canSeek() )
        {
            long int pos = position();
            if( seek(0, SEEK_END) )
            {
                len = position();
            }
            seek(pos, SEEK_SET);
        }
        return len;
    }


    long int FileStream::position()
    {
        if( !_file )
            return -1;
        return ftell(_file);
    }


    bool FileStream::seek(long int offset, int origin)
    {
        if( !_file )
            return false;
        return fseek(_file, offset, origin) == 0;
    }


    bool FileStream::rewind()
    {
        if( canSeek() )
        {
            ::rewind(_file);
            return true;
        }
        return false;
    }
}
