#include "Base.h"
#include "Properties.h"
#include "FileSystem.h"

#include <glm/gtc/type_ptr.hpp>

#include <boost/log/trivial.hpp>


namespace gameplay
{
    /**
     * Reads the next character from the stream. Returns EOF if the end of the stream is reached.
     */
    static signed char readChar(Stream* stream)
    {
        if( stream->eof() )
            return EOF;
        signed char c;
        if( stream->read(&c, 1, 1) != 1 )
            return EOF;
        return c;
    }


    // Utility functions (shared with SceneLoader).
    /** @script{ignore} */
    void calculateNamespacePath(const std::string& urlString, std::string& fileString, std::vector<std::string>& namespacePath);
    /** @script{ignore} */
    Properties* getPropertiesFromNamespacePath(Properties* properties, const std::vector<std::string>& namespacePath);


    Properties::Properties()
        : _variables(nullptr)
        , _dirPath(nullptr)
        , _parent(nullptr)
    {
    }


    Properties::Properties(const Properties& copy)
        : _namespace(copy._namespace)
        , _id(copy._id)
        , _parentID(copy._parentID)
        , _properties(copy._properties)
        , _variables(nullptr)
        , _dirPath(nullptr)
        , _parent(copy._parent)
    {
        setDirectoryPath(copy._dirPath);
        _namespaces = std::vector<Properties*>();
        std::vector<Properties*>::const_iterator it;
        for( it = copy._namespaces.begin(); it < copy._namespaces.end(); ++it )
        {
            BOOST_ASSERT(*it);
            _namespaces.push_back(new Properties(**it));
        }
        rewind();
    }


    Properties::Properties(Stream* stream)
        : _variables(nullptr)
        , _dirPath(nullptr)
        , _parent(nullptr)
    {
        readProperties(stream);
        rewind();
    }


    Properties::Properties(Stream* stream, const char* name, const char* id, const char* parentID, Properties* parent)
        : _namespace(name)
        , _variables(nullptr)
        , _dirPath(nullptr)
        , _parent(parent)
    {
        if( id )
        {
            _id = id;
        }
        if( parentID )
        {
            _parentID = parentID;
        }
        readProperties(stream);
        rewind();
    }


    Properties* Properties::create(const char* url)
    {
        if( !url || strlen(url) == 0 )
        {
            BOOST_LOG_TRIVIAL(error) << "Attempting to create a Properties object from an empty URL!";
            return nullptr;
        }

        // Calculate the file and full namespace path from the specified url.
        std::string urlString = url;
        std::string fileString;
        std::vector<std::string> namespacePath;
        calculateNamespacePath(urlString, fileString, namespacePath);

        std::unique_ptr<Stream> stream(FileSystem::open(fileString.c_str()));
        if( stream.get() == nullptr )
        {
            BOOST_LOG_TRIVIAL(error) << "Failed to open file '" << fileString << "'.";
            return nullptr;
        }

        Properties* properties = new Properties(stream.get());
        properties->resolveInheritance();
        stream->close();

        // Get the specified properties object.
        Properties* p = getPropertiesFromNamespacePath(properties, namespacePath);
        if( !p )
        {
            BOOST_LOG_TRIVIAL(warning) << "Failed to load properties from url '" << url << "'.";
            SAFE_DELETE(properties);
            return nullptr;
        }

        // If the loaded properties object is not the root namespace,
        // then we have to clone it and delete the root namespace
        // so that we don't leak memory.
        if( p != properties )
        {
            p = p->clone();
            SAFE_DELETE(properties);
        }
        p->setDirectoryPath(FileSystem::getDirectoryName(fileString.c_str()));
        return p;
    }


    static bool isVariable(const char* str, char* outName, size_t outSize)
    {
        size_t len = strlen(str);
        if( len > 3 && str[0] == '$' && str[1] == '{' && str[len - 1] == '}' )
        {
            size_t size = len - 3;
            if( size > (outSize - 1) )
                size = outSize - 1;
            strncpy(outName, str + 2, len - 3);
            outName[len - 3] = 0;
            return true;
        }

        return false;
    }


    void Properties::readProperties(Stream* stream)
    {
        BOOST_ASSERT(stream);

        char line[2048];
        char variable[256];
        int c;
        char* name;
        char* value;
        char* parentID;
        char* rc;
        char* rcc;
        char* rccc;
        bool comment = false;

        while( true )
        {
            // Skip whitespace at the start of lines
            skipWhiteSpace(stream);

            // Stop when we have reached the end of the file.
            if( stream->eof() )
                break;

            // Read the next line.
            rc = stream->readLine(line, 2048);
            if( rc == nullptr )
            {
                BOOST_LOG_TRIVIAL(error) << "Error reading line from file.";
                return;
            }

            // Ignore comments
            if( comment )
            {
                // Check for end of multi-line comment at either start or end of line
                if( strncmp(line, "*/", 2) == 0 )
                    comment = false;
                else
                {
                    trimWhiteSpace(line);
                    const int len = strlen(line);
                    if( len >= 2 && strncmp(line + (len - 2), "*/", 2) == 0 )
                        comment = false;
                }
            }
            else if( strncmp(line, "/*", 2) == 0 )
            {
                // Start of multi-line comment (must be at start of line)
                comment = true;
            }
            else if( strncmp(line, "//", 2) != 0 )
            {
                // If an '=' appears on this line, parse it as a name/value pair.
                // Note: strchr() has to be called before strtok(), or a backup of line has to be kept.
                rc = strchr(line, '=');
                if( rc != nullptr )
                {
                    // First token should be the property name.
                    name = strtok(line, "=");
                    if( name == nullptr )
                    {
                        BOOST_LOG_TRIVIAL(error) << "Error parsing properties file: attribute without name.";
                        return;
                    }

                    // Remove white-space from name.
                    name = trimWhiteSpace(name);

                    // Scan for next token, the property's value.
                    value = strtok(nullptr, "");
                    if( value == nullptr )
                    {
                        BOOST_LOG_TRIVIAL(error) << "Error parsing properties file: attribute with name ('" << name << "') but no value.";
                        return;
                    }

                    // Remove white-space from value.
                    value = trimWhiteSpace(value);

                    // Is this a variable assignment?
                    if( isVariable(name, variable, 256) )
                    {
                        setVariable(variable, value);
                    }
                    else
                    {
                        // Normal name/value pair
                        _properties.push_back(Property(name, value));
                    }
                }
                else
                {
                    parentID = nullptr;

                    // Get the last character on the line (ignoring whitespace).
                    const char* lineEnd = trimWhiteSpace(line) + (strlen(trimWhiteSpace(line)) - 1);

                    // This line might begin or end a namespace,
                    // or it might be a key/value pair without '='.

                    // Check for '{' on same line.
                    rc = strchr(line, '{');

                    // Check for inheritance: ':'
                    rcc = strchr(line, ':');

                    // Check for '}' on same line.
                    rccc = strchr(line, '}');

                    // Get the name of the namespace.
                    name = strtok(line, " \t\n{");
                    name = trimWhiteSpace(name);
                    if( name == nullptr )
                    {
                        BOOST_LOG_TRIVIAL(error) << "Error parsing properties file: failed to determine a valid token for line '" << line << "'.";
                        return;
                    }
                    else if( name[0] == '}' )
                    {
                        // End of namespace.
                        return;
                    }

                    // Get its ID if it has one.
                    value = strtok(nullptr, ":{");
                    value = trimWhiteSpace(value);

                    // Get its parent ID if it has one.
                    if( rcc != nullptr )
                    {
                        parentID = strtok(nullptr, "{");
                        parentID = trimWhiteSpace(parentID);
                    }

                    if( value != nullptr && value[0] == '{' )
                    {
                        // If the namespace ends on this line, seek back to right before the '}' character.
                        if( rccc && rccc == lineEnd )
                        {
                            if( stream->seek(-1, SEEK_CUR) == false )
                            {
                                BOOST_LOG_TRIVIAL(error) << "Failed to seek back to before a '}' character in properties file.";
                                return;
                            }
                            while( readChar(stream) != '}' )
                            {
                                if( stream->seek(-2, SEEK_CUR) == false )
                                {
                                    BOOST_LOG_TRIVIAL(error) << "Failed to seek back to before a '}' character in properties file.";
                                    return;
                                }
                            }
                            if( stream->seek(-1, SEEK_CUR) == false )
                            {
                                BOOST_LOG_TRIVIAL(error) << "Failed to seek back to before a '}' character in properties file.";
                                return;
                            }
                        }

                        // New namespace without an ID.
                        Properties* space = new Properties(stream, name, nullptr, parentID, this);
                        _namespaces.push_back(space);

                        // If the namespace ends on this line, seek to right after the '}' character.
                        if( rccc && rccc == lineEnd )
                        {
                            if( stream->seek(1, SEEK_CUR) == false )
                            {
                                BOOST_LOG_TRIVIAL(error) << "Failed to seek to immediately after a '}' character in properties file.";
                                return;
                            }
                        }
                    }
                    else
                    {
                        // If '{' appears on the same line.
                        if( rc != nullptr )
                        {
                            // If the namespace ends on this line, seek back to right before the '}' character.
                            if( rccc && rccc == lineEnd )
                            {
                                if( stream->seek(-1, SEEK_CUR) == false )
                                {
                                    BOOST_LOG_TRIVIAL(error) << "Failed to seek back to before a '}' character in properties file.";
                                    return;
                                }
                                while( readChar(stream) != '}' )
                                {
                                    if( stream->seek(-2, SEEK_CUR) == false )
                                    {
                                        BOOST_LOG_TRIVIAL(error) << "Failed to seek back to before a '}' character in properties file.";
                                        return;
                                    }
                                }
                                if( stream->seek(-1, SEEK_CUR) == false )
                                {
                                    BOOST_LOG_TRIVIAL(error) << "Failed to seek back to before a '}' character in properties file.";
                                    return;
                                }
                            }

                            // Create new namespace.
                            Properties* space = new Properties(stream, name, value, parentID, this);
                            _namespaces.push_back(space);

                            // If the namespace ends on this line, seek to right after the '}' character.
                            if( rccc && rccc == lineEnd )
                            {
                                if( stream->seek(1, SEEK_CUR) == false )
                                {
                                    BOOST_LOG_TRIVIAL(error) << "Failed to seek to immediately after a '}' character in properties file.";
                                    return;
                                }
                            }
                        }
                        else
                        {
                            // Find out if the next line starts with "{"
                            skipWhiteSpace(stream);
                            c = readChar(stream);
                            if( c == '{' )
                            {
                                // Create new namespace.
                                Properties* space = new Properties(stream, name, value, parentID, this);
                                _namespaces.push_back(space);
                            }
                            else
                            {
                                // Back up from fgetc()
                                if( stream->seek(-1, SEEK_CUR) == false )
                                BOOST_LOG_TRIVIAL(error) << "Failed to seek backwards a single character after testing if the next line starts with '{'.";

                                // Store "name value" as a name/value pair, or even just "name".
                                if( value != nullptr )
                                {
                                    _properties.push_back(Property(name, value));
                                }
                                else
                                {
                                    _properties.push_back(Property(name, ""));
                                }
                            }
                        }
                    }
                }
            }
        }
    }


    Properties::~Properties()
    {
        SAFE_DELETE(_dirPath);
        for( size_t i = 0, count = _namespaces.size(); i < count; ++i )
        {
            SAFE_DELETE(_namespaces[i]);
        }

        SAFE_DELETE(_variables);
    }


    void Properties::skipWhiteSpace(Stream* stream)
    {
        signed char c;
        do
        {
            c = readChar(stream);
        } while( isspace(c) && c != EOF );

        // If we are not at the end of the file, then since we found a
        // non-whitespace character, we put the cursor back in front of it.
        if( c != EOF )
        {
            if( stream->seek(-1, SEEK_CUR) == false )
            {
                BOOST_LOG_TRIVIAL(error) << "Failed to seek backwards one character after skipping whitespace.";
            }
        }
    }


    char* Properties::trimWhiteSpace(char* str)
    {
        if( str == nullptr )
        {
            return str;
        }

        char* end;

        // Trim leading space.
        while( isspace(*str) )
            str++;

        // All spaces?
        if( *str == 0 )
        {
            return str;
        }

        // Trim trailing space.
        end = str + strlen(str) - 1;
        while( end > str && isspace(*end) )
            end--;

        // Write new null terminator.
        *(end + 1) = 0;

        return str;
    }


    void Properties::resolveInheritance(const char* id)
    {
        // Namespaces can be defined like so:
        // "name id : parentID { }"
        // This method merges data from the parent namespace into the child.

        // Get a top-level namespace.
        Properties* derived;
        if( id )
        {
            derived = getNamespace(id);
        }
        else
        {
            derived = getNextNamespace();
        }
        while( derived )
        {
            // If the namespace has a parent ID, find the parent.
            if( !derived->_parentID.empty() )
            {
                Properties* parent = getNamespace(derived->_parentID.c_str());
                if( parent )
                {
                    resolveInheritance(parent->getId());

                    // Copy the child.
                    Properties* overrides = new Properties(*derived);

                    // Delete the child's data.
                    for( size_t i = 0, count = derived->_namespaces.size(); i < count; i++ )
                    {
                        SAFE_DELETE(derived->_namespaces[i]);
                    }

                    // Copy data from the parent into the child.
                    derived->_properties = parent->_properties;
                    derived->_namespaces = std::vector<Properties*>();
                    std::vector<Properties*>::const_iterator itt;
                    for( itt = parent->_namespaces.begin(); itt < parent->_namespaces.end(); ++itt )
                    {
                        BOOST_ASSERT(*itt);
                        derived->_namespaces.push_back(new Properties(**itt));
                    }
                    derived->rewind();

                    // Take the original copy of the child and override the data copied from the parent.
                    derived->mergeWith(overrides);

                    // Delete the child copy.
                    SAFE_DELETE(overrides);
                }
            }

            // Resolve inheritance within this namespace.
            derived->resolveInheritance();

            // Get the next top-level namespace and check again.
            if( !id )
            {
                derived = getNextNamespace();
            }
            else
            {
                derived = nullptr;
            }
        }
    }


    void Properties::mergeWith(Properties* overrides)
    {
        BOOST_ASSERT(overrides);

        // Overwrite or add each property found in child.
        overrides->rewind();
        const char* name = overrides->getNextProperty();
        while( name )
        {
            this->setString(name, overrides->getString());
            name = overrides->getNextProperty();
        }
        this->_propertiesItr = this->_properties.end();

        // Merge all common nested namespaces, add new ones.
        Properties* overridesNamespace = overrides->getNextNamespace();
        while( overridesNamespace )
        {
            bool merged = false;

            rewind();
            Properties* derivedNamespace = getNextNamespace();
            while( derivedNamespace )
            {
                if( strcmp(derivedNamespace->getNamespace(), overridesNamespace->getNamespace()) == 0 &&
                    strcmp(derivedNamespace->getId(), overridesNamespace->getId()) == 0 )
                {
                    derivedNamespace->mergeWith(overridesNamespace);
                    merged = true;
                }

                derivedNamespace = getNextNamespace();
            }

            if( !merged )
            {
                // Add this new namespace.
                Properties* newNamespace = new Properties(*overridesNamespace);

                this->_namespaces.push_back(newNamespace);
                this->_namespacesItr = this->_namespaces.end();
            }

            overridesNamespace = overrides->getNextNamespace();
        }
    }


    const char* Properties::getNextProperty()
    {
        if( _propertiesItr == _properties.end() )
        {
            // Restart from the beginning
            _propertiesItr = _properties.begin();
        }
        else
        {
            // Move to the next property
            ++_propertiesItr;
        }

        return _propertiesItr == _properties.end() ? nullptr : _propertiesItr->name.c_str();
    }


    Properties* Properties::getNextNamespace()
    {
        if( _namespacesItr == _namespaces.end() )
        {
            // Restart from the beginning
            _namespacesItr = _namespaces.begin();
        }
        else
        {
            ++_namespacesItr;
        }

        if( _namespacesItr != _namespaces.end() )
        {
            Properties* ns = *_namespacesItr;
            return ns;
        }

        return nullptr;
    }


    void Properties::rewind()
    {
        _propertiesItr = _properties.end();
        _namespacesItr = _namespaces.end();
    }


    Properties* Properties::getNamespace(const char* id, bool searchNames, bool recurse) const
    {
        BOOST_ASSERT(id);

        for( std::vector<Properties*>::const_iterator it = _namespaces.begin(); it < _namespaces.end(); ++it )
        {
            Properties* p = *it;
            if( strcmp(searchNames ? p->_namespace.c_str() : p->_id.c_str(), id) == 0 )
                return p;

            if( recurse )
            {
                // Search recursively.
                p = p->getNamespace(id, searchNames, true);
                if( p )
                    return p;
            }
        }

        return nullptr;
    }


    const char* Properties::getNamespace() const
    {
        return _namespace.c_str();
    }


    const char* Properties::getId() const
    {
        return _id.c_str();
    }


    bool Properties::exists(const char* name) const
    {
        if( name == nullptr )
            return false;

        for( std::list<Property>::const_iterator itr = _properties.begin(); itr != _properties.end(); ++itr )
        {
            if( itr->name == name )
                return true;
        }

        return false;
    }


    static bool isStringNumeric(const char* str)
    {
        BOOST_ASSERT(str);

        // The first character may be '-'
        if( *str == '-' )
            str++;

        // The first character after the sign must be a digit
        if( !isdigit(*str) )
            return false;
        str++;

        // All remaining characters must be digits, with a single decimal (.) permitted
        unsigned int decimalCount = 0;
        while( *str )
        {
            if( !isdigit(*str) )
            {
                if( *str == '.' && decimalCount == 0 )
                {
                    // Max of 1 decimal allowed
                    decimalCount++;
                }
                else
                {
                    return false;
                }
            }
            str++;
        }
        return true;
    }


    Properties::Type Properties::getType(const char* name) const
    {
        const char* value = getString(name);
        if( !value )
        {
            return Properties::NONE;
        }

        // Parse the value to determine the format
        unsigned int commaCount = 0;
        char* valuePtr = const_cast<char*>(value);
        while( (valuePtr = strchr(valuePtr, ',')) )
        {
            valuePtr++;
            commaCount++;
        }

        switch( commaCount )
        {
            case 0:
                return isStringNumeric(value) ? Properties::NUMBER : Properties::STRING;
            case 1:
                return Properties::VECTOR2;
            case 2:
                return Properties::VECTOR3;
            case 3:
                return Properties::VECTOR4;
            case 15:
                return Properties::MATRIX;
            default:
                return Properties::STRING;
        }
    }


    const char* Properties::getString(const char* name, const char* defaultValue) const
    {
        char variable[256];
        const char* value = nullptr;

        if( name )
        {
            // If 'name' is a variable, return the variable value
            if( isVariable(name, variable, 256) )
            {
                return getVariable(variable, defaultValue);
            }

            for( std::list<Property>::const_iterator itr = _properties.begin(); itr != _properties.end(); ++itr )
            {
                if( itr->name == name )
                {
                    value = itr->value.c_str();
                    break;
                }
            }
        }
        else
        {
            // No name provided - get the value at the current iterator position
            if( _propertiesItr != _properties.end() )
            {
                value = _propertiesItr->value.c_str();
            }
        }

        if( value )
        {
            // If the value references a variable, return the variable value
            if( isVariable(value, variable, 256) )
                return getVariable(variable, defaultValue);

            return value;
        }

        return defaultValue;
    }


    bool Properties::setString(const char* name, const char* value)
    {
        if( name )
        {
            for( std::list<Property>::iterator itr = _properties.begin(); itr != _properties.end(); ++itr )
            {
                if( itr->name == name )
                {
                    // Update the first property that matches this name
                    itr->value = value ? value : "";
                    return true;
                }
            }

            // There is no property with this name, so add one
            _properties.push_back(Property(name, value ? value : ""));
        }
        else
        {
            // If there's a current property, set its value
            if( _propertiesItr == _properties.end() )
                return false;

            _propertiesItr->value = value ? value : "";
        }

        return true;
    }


    bool Properties::getBool(const char* name, bool defaultValue) const
    {
        const char* valueString = getString(name);
        if( valueString )
        {
            return (strcmp(valueString, "true") == 0);
        }

        return defaultValue;
    }


    int Properties::getInt(const char* name) const
    {
        const char* valueString = getString(name);
        if( valueString )
        {
            int value;
            int scanned;
            scanned = sscanf(valueString, "%d", &value);
            if( scanned != 1 )
            {
                BOOST_LOG_TRIVIAL(error) << "Error attempting to parse property '" << name << "' as an integer.";
                return 0;
            }
            return value;
        }

        return 0;
    }


    float Properties::getFloat(const char* name) const
    {
        const char* valueString = getString(name);
        if( valueString )
        {
            float value;
            int scanned;
            scanned = sscanf(valueString, "%f", &value);
            if( scanned != 1 )
            {
                BOOST_LOG_TRIVIAL(error) << "Error attempting to parse property '" << name << "' as a float.";
                return 0.0f;
            }
            return value;
        }

        return 0.0f;
    }


    long Properties::getLong(const char* name) const
    {
        const char* valueString = getString(name);
        if( valueString )
        {
            long value;
            int scanned;
            scanned = sscanf(valueString, "%ld", &value);
            if( scanned != 1 )
            {
                BOOST_LOG_TRIVIAL(error) << "Error attempting to parse property '" << name << "' as a long integer.";
                return 0L;
            }
            return value;
        }

        return 0L;
    }


    bool Properties::getMatrix(const char* name, glm::mat4* out) const
    {
        BOOST_ASSERT(out);

        const char* valueString = getString(name);
        if( valueString )
        {
            float m[16];
            int scanned;
            scanned = sscanf(valueString, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f",
                             &m[0], &m[1], &m[2], &m[3], &m[4], &m[5], &m[6], &m[7],
                             &m[8], &m[9], &m[10], &m[11], &m[12], &m[13], &m[14], &m[15]);

            if( scanned != 16 )
            {
                BOOST_LOG_TRIVIAL(error) << "Error attempting to parse property '" << name << "' as a matrix.";
                *out = glm::mat4(1);
                return false;
            }

            std::copy_n(m, 16, glm::value_ptr(*out));
            return true;
        }

        *out = glm::mat4(1);
        return false;
    }


    bool Properties::getVector2(const char* name, glm::vec2* out) const
    {
        return parseVector2(getString(name), out);
    }


    bool Properties::getVector3(const char* name, glm::vec3* out) const
    {
        return parseVector3(getString(name), out);
    }


    bool Properties::getVector4(const char* name, glm::vec4* out) const
    {
        return parseVector4(getString(name), out);
    }


    bool Properties::getQuaternionFromAxisAngle(const char* name, glm::quat* out) const
    {
        return parseAxisAngle(getString(name), out);
    }


    bool Properties::getColor(const char* name, glm::vec3* out) const
    {
        return parseColor(getString(name), out);
    }


    bool Properties::getColor(const char* name, glm::vec4* out) const
    {
        return parseColor(getString(name), out);
    }


    bool Properties::getPath(const char* name, std::string* path) const
    {
        BOOST_ASSERT(name && path);
        const char* valueString = getString(name);
        if( valueString )
        {
            if( FileSystem::fileExists(valueString) )
            {
                path->assign(valueString);
                return true;
            }
            else
            {
                const Properties* prop = this;
                while( prop != nullptr )
                {
                    // Search for the file path relative to the bundle file
                    const std::string* dirPath = prop->_dirPath;
                    if( dirPath != nullptr && !dirPath->empty() )
                    {
                        std::string relativePath = *dirPath;
                        relativePath.append(valueString);
                        if( FileSystem::fileExists(relativePath.c_str()) )
                        {
                            path->assign(relativePath);
                            return true;
                        }
                    }
                    prop = prop->_parent;
                }
            }
        }
        return false;
    }


    const char* Properties::getVariable(const char* name, const char* defaultValue) const
    {
        if( name == nullptr )
            return defaultValue;

        // Search for variable in this Properties object
        if( _variables )
        {
            for( size_t i = 0, count = _variables->size(); i < count; ++i )
            {
                Property& prop = (*_variables)[i];
                if( prop.name == name )
                    return prop.value.c_str();
            }
        }

        // Search for variable in parent Properties
        return _parent ? _parent->getVariable(name, defaultValue) : defaultValue;
    }


    void Properties::setVariable(const char* name, const char* value)
    {
        BOOST_ASSERT(name);

        Property* prop = nullptr;

        // Search for variable in this Properties object and parents
        Properties* current = const_cast<Properties*>(this);
        while( current )
        {
            if( current->_variables )
            {
                for( size_t i = 0, count = current->_variables->size(); i < count; ++i )
                {
                    Property* p = &(*current->_variables)[i];
                    if( p->name == name )
                    {
                        prop = p;
                        break;
                    }
                }
            }
            current = current->_parent;
        }

        if( prop )
        {
            // Found an existing property, set it
            prop->value = value ? value : "";
        }
        else
        {
            // Add a new variable with this name
            if( !_variables )
                _variables = new std::vector<Property>();
            _variables->push_back(Property(name, value ? value : ""));
        }
    }


    Properties* Properties::clone()
    {
        Properties* p = new Properties();

        p->_namespace = _namespace;
        p->_id = _id;
        p->_parentID = _parentID;
        p->_properties = _properties;
        p->_propertiesItr = p->_properties.end();
        p->setDirectoryPath(_dirPath);

        for( size_t i = 0, count = _namespaces.size(); i < count; i++ )
        {
            BOOST_ASSERT(_namespaces[i]);
            Properties* child = _namespaces[i]->clone();
            p->_namespaces.push_back(child);
            child->_parent = p;
        }
        p->_namespacesItr = p->_namespaces.end();

        return p;
    }


    void Properties::setDirectoryPath(const std::string* path)
    {
        if( path )
        {
            setDirectoryPath(*path);
        }
        else
        {
            SAFE_DELETE(_dirPath);
        }
    }


    void Properties::setDirectoryPath(const std::string& path)
    {
        if( _dirPath == nullptr )
        {
            _dirPath = new std::string(path);
        }
        else
        {
            _dirPath->assign(path);
        }
    }


    void calculateNamespacePath(const std::string& urlString, std::string& fileString, std::vector<std::string>& namespacePath)
    {
        // If the url references a specific namespace within the file,
        // calculate the full namespace path to the final namespace.
        size_t loc = urlString.rfind("#");
        if( loc != std::string::npos )
        {
            fileString = urlString.substr(0, loc);
            std::string namespacePathString = urlString.substr(loc + 1);
            while( (loc = namespacePathString.find("/")) != std::string::npos )
            {
                namespacePath.push_back(namespacePathString.substr(0, loc));
                namespacePathString = namespacePathString.substr(loc + 1);
            }
            namespacePath.push_back(namespacePathString);
        }
        else
        {
            fileString = urlString;
        }
    }


    Properties* getPropertiesFromNamespacePath(Properties* properties, const std::vector<std::string>& namespacePath)
    {
        // If the url references a specific namespace within the file,
        // return the specified namespace or notify the user if it cannot be found.
        if( namespacePath.size() > 0 )
        {
            size_t size = namespacePath.size();
            properties->rewind();
            Properties* iter = properties->getNextNamespace();
            for( size_t i = 0; i < size; )
            {
                while( true )
                {
                    if( iter == nullptr )
                    {
                        BOOST_LOG_TRIVIAL(warning) << "Failed to load properties object from url.";
                        return nullptr;
                    }

                    if( strcmp(iter->getId(), namespacePath[i].c_str()) == 0 )
                    {
                        if( i != size - 1 )
                        {
                            properties = iter->getNextNamespace();
                            iter = properties;
                        }
                        else
                            properties = iter;

                        i++;
                        break;
                    }

                    iter = properties->getNextNamespace();
                }
            }

            return properties;
        }
        else
            return properties;
    }


    bool Properties::parseVector2(const char* str, glm::vec2* out)
    {
        if( str )
        {
            float x, y;
            if( sscanf(str, "%f,%f", &x, &y) == 2 )
            {
                if(out)
                    *out = { x,y };
                return true;
            }
            else
            {
                BOOST_LOG_TRIVIAL(warning) << "Error attempting to parse property as a two-dimensional vector: " << str;
            }
        }

        if(out)
            *out = { 0,0 };
        return false;
    }


    bool Properties::parseVector3(const char* str, glm::vec3* out)
    {
        if( str )
        {
            float x, y, z;
            if( sscanf(str, "%f,%f,%f", &x, &y, &z) == 3 )
            {
                if(out)
                    *out = { 0,0,0 };
                return true;
            }
            else
            {
                BOOST_LOG_TRIVIAL(warning) << "Error attempting to parse property as a three-dimensional vector: " << str;
            }
        }

        if(out)
            *out = { 0,0,0 };
        return false;
    }


    bool Properties::parseVector4(const char* str, glm::vec4* out)
    {
        if( str )
        {
            float x, y, z, w;
            if( sscanf(str, "%f,%f,%f,%f", &x, &y, &z, &w) == 4 )
            {
                if( out )
                    *out = { x,y,z,w };
                return true;
            }
            else
            {
                BOOST_LOG_TRIVIAL(warning) << "Error attempting to parse property as a four-dimensional vector: " << str;
            }
        }

        if( out )
            *out = { 0,0,0,0 };
        return false;
    }


    bool Properties::parseAxisAngle(const char* str, glm::quat* out)
    {
        if( str )
        {
            float x, y, z, theta;
            if( sscanf(str, "%f,%f,%f,%f", &x, &y, &z, &theta) == 4 )
            {
                if( out )
                    *out = glm::quat{ MATH_DEG_TO_RAD(theta), glm::vec3(x, y, z) };
                return true;
            }
            else
            {
                BOOST_LOG_TRIVIAL(warning) << "Error attempting to parse property as an axis-angle rotation: " << str;
            }
        }

        if( out )
            *out = glm::quat();
        return false;
    }


    bool Properties::parseColor(const char* str, glm::vec3* out)
    {
        if( str )
        {
            if( strlen(str) == 7 && str[0] == '#' )
            {
                // Read the string into an int as hex.
                unsigned int color[3];
                if( sscanf(str + 1, "%02x%02x%02x", &color[0], &color[1], &color[2]) == 3 )
                {
                    if(out)
                        *out = { color[0] / 255.0f, color[1] / 255.0f, color[2] / 255.0f };
                    return true;
                }
                else
                {
                    // Invalid format
                    BOOST_LOG_TRIVIAL(error) << "Error attempting to parse property as an RGB color: " << str;
                }
            }
            else
            {
                // Not a color string.
                BOOST_LOG_TRIVIAL(error) << "Error attempting to parse property as an RGB color (not specified as a color string): " << str;
            }
        }

        if(out)
            *out = { 0,0,0 };
        return false;
    }


    bool Properties::parseColor(const char* str, glm::vec4* out)
    {
        if( str )
        {
            if( strlen(str) == 9 && str[0] == '#' )
            {
                // Read the string into an int as hex.
                unsigned int color[4];
                if(sscanf(str + 1, "%02x%02x%02x%02x", &color[0], &color[1], &color[2], &color[3]) == 4)
                {
                    if(out)
                        *out = { color[0] / 255.0f, color[1] / 255.0f, color[2] / 255.0f,color[3] / 255.0f };
                    return true;
                }
                else
                {
                    // Invalid format
                    BOOST_LOG_TRIVIAL(warning) << "Error attempting to parse property as an RGBA color: " << str;
                }
            }
            else
            {
                // Not a color string.
                BOOST_LOG_TRIVIAL(warning) << "Error attempting to parse property as an RGBA color (not specified as a color string): " << str;
            }
        }

        if(out)
            *out = { 0,0,0,0 };
        return false;
    }
}
