#include "Base.h"
#include "FileSystem.h"
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
    /**
     *
     * @script{ignore}
     */
    class FileStream : public Stream
    {
    public:
        friend class FileSystem;

        ~FileStream();
        bool canRead() override;
        bool canWrite() override;
        bool canSeek() override;
        void close() override;
        size_t read(void* ptr, size_t size, size_t count) override;
        char* readLine(char* str, int num) override;
        size_t write(const void* ptr, size_t size, size_t count) override;
        bool eof() override;
        size_t length() override;
        long int position() override;
        bool seek(long int offset, int origin) override;
        bool rewind() override;

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


    Stream* FileSystem::open(const char* path, size_t streamMode)
    {
        char modeStr[] = "rb";
        if( (streamMode & WRITE) != 0 )
            modeStr[0] = 'w';

        return FileStream::create(path, modeStr);
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
            return nullptr;
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
        return static_cast<size_t>(position()) >= length();
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
