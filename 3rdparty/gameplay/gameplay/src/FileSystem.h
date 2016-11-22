#pragma once

#include "Stream.h"

#include <vector>


namespace gameplay
{
    /**
     * Defines a set of functions for interacting with the device file system.
     */
    class FileSystem
    {
    public:

        /**
         * Mode flags for opening a stream.
         *
         * @script{ignore}
         */
        enum StreamMode
        {
            READ = 1,
            WRITE = 2
        };


        ~FileSystem();

        /**
         * Opens a byte stream for the given resource path.
         *
         * If <code>path</code> is a file path, the file at the specified location is opened relative to the currently set
         * resource path.
         *
         * @param path The path to the resource to be opened, relative to the currently set resource path.
         * @param streamMode The stream mode used to open the file.
         *
         * @return A stream that can be used to read or write to the file depending on the mode.
         *         Returns NULL if there was an error. (Request mode not supported).
         *
         * @script{ignore}
         */
        static Stream* open(const char* path, size_t streamMode = READ);

        /**
         * Reads the entire contents of the specified file and returns its contents.
         *
         * The returned character array is allocated with new[] and must therefore
         * deleted by the caller using delete[].
         *
         * @param filePath The path to the file to be read.
         * @param fileSize The size of the file in bytes (optional).
         *
         * @return A newly allocated (NULL-terminated) character array containing the
         *      contents of the file, or NULL if the file could not be read.
         */
        static std::string readAll(const std::string& filePath, size_t* fileSize = nullptr);

    private:

        /**
         * Constructor.
         */
        FileSystem();
    };
}
