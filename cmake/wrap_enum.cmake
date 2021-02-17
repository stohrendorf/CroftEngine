function( wrap_enum )
    cmake_parse_arguments(
            WRAP_ENUM
            "" # no options
            "NAMESPACE;NAME;TYPE;SOURCE;DEST"
            "WITH_INCLUDES"
            ${ARGN}
    )

    if( WRAP_ENUM_UNPARSED_ARGUMENTS )
        message( FATAL_ERROR "Unexpected ${WRAP_ENUM_UNPARSED_ARGUMENTS}" )
    endif()
    if( WRAP_ENUM_KEYWORDS_MISSING_VALUES )
        message( FATAL_ERROR "Missing ${WRAP_ENUM_KEYWORDS_MISSING_VALUES}" )
    endif()

    if( EXISTS "${WRAP_ENUM_DEST}" AND "${WRAP_ENUM_DEST}" IS_NEWER_THAN "${WRAP_ENUM_SOURCE}" )
        return()
    endif()

    message( STATUS "Generating enum include ${WRAP_ENUM_DEST} from ${WRAP_ENUM_SOURCE}" )
    file( STRINGS "${WRAP_ENUM_SOURCE}" _vals )
    file( WRITE "${WRAP_ENUM_DEST}" "" )

    # headers
    file( APPEND "${WRAP_ENUM_DEST}" "#pragma once\n" )
    file( APPEND "${WRAP_ENUM_DEST}" "\n" )
    file( APPEND "${WRAP_ENUM_DEST}" "#include <cstdint>\n" )
    file( APPEND "${WRAP_ENUM_DEST}" "#include <gsl-lite.hpp>\n" )
    file( APPEND "${WRAP_ENUM_DEST}" "#include <map>\n" )
    file( APPEND "${WRAP_ENUM_DEST}" "#include <stdexcept>\n" )
    file( APPEND "${WRAP_ENUM_DEST}" "#include <string>\n" )
    foreach( _include IN LISTS WRAP_ENUM_WITH_INCLUDES )
        if( NOT ${_include} MATCHES "^<" )
            set( _include "\"${include}\"" )
        endif()
        file( APPEND "${WRAP_ENUM_DEST}" "#include ${_include}\n" )
    endforeach()
    file( APPEND "${WRAP_ENUM_DEST}" "\n" )

    # Write enum
    file( APPEND "${WRAP_ENUM_DEST}" "namespace ${WRAP_ENUM_NAMESPACE}\n" )
    file( APPEND "${WRAP_ENUM_DEST}" "{\n" )
    file( APPEND "${WRAP_ENUM_DEST}" "enum class ${WRAP_ENUM_NAME} : ${WRAP_ENUM_TYPE}\n" )
    file( APPEND "${WRAP_ENUM_DEST}" "{\n" )
    foreach( _val IN LISTS _vals )
        # remove trailing whitespace/comments
        string( REGEX REPLACE "//.*\$" "" _val "${_val}" )
        string( REPLACE " " "" _val "${_val}" )
        string( REPLACE "\t" "" _val "${_val}" )
        if( _val STREQUAL "" )
            continue()
        endif()

        # remove "= 123" part
        string( REGEX REPLACE "=.+\$" "" _ename "${_val}" )
        if( _ename STREQUAL "" )
            continue()
        endif()
        if( _val MATCHES ".+=.+" )
            # extract value of "= 123" part
            string( REGEX REPLACE "^.+=(.+)" "\\1" _eval "${_val}" )
            # emit: Value = IntValue,
            file( APPEND "${WRAP_ENUM_DEST}" "    ${_ename} = ${_eval},\n" )
        else()
            # emit: Value,
            file( APPEND "${WRAP_ENUM_DEST}" "    ${_ename},\n" )
        endif()
    endforeach()
    file( APPEND "${WRAP_ENUM_DEST}" "};\n\n" )

    # toString function
    file( APPEND "${WRAP_ENUM_DEST}" "inline gsl::czstring toString(${WRAP_ENUM_NAME} val)\n" )
    file( APPEND "${WRAP_ENUM_DEST}" "{\n" )
    file( APPEND "${WRAP_ENUM_DEST}" "    switch(val)\n" )
    file( APPEND "${WRAP_ENUM_DEST}" "    {\n" )
    foreach( _val IN LISTS _vals )
        # remove trailing whitespace/comments
        string( REGEX REPLACE "//.*\$" "" _val "${_val}" )
        string( REPLACE " " "" _val "${_val}" )
        string( REPLACE "\t" "" _val "${_val}" )
        if( _val STREQUAL "" )
            continue()
        endif()
        # remove "= 123" part
        string( REGEX REPLACE "=.+\$" "" _ename "${_val}" )
        if( _ename STREQUAL "" )
            continue()
        endif()
        # emit: case Enum::Value: return "Value";
        file( APPEND "${WRAP_ENUM_DEST}" "        case ${WRAP_ENUM_NAME}::${_ename}: return \"${_ename}\";\n" )
    endforeach()
    file( APPEND "${WRAP_ENUM_DEST}" "        default: return nullptr;\n" )
    file( APPEND "${WRAP_ENUM_DEST}" "    }\n" )
    file( APPEND "${WRAP_ENUM_DEST}" "}\n\n" )

    # EnumUtil forward declaration
    file( APPEND "${WRAP_ENUM_DEST}" "template<typename T> struct EnumUtil;\n\n" )
    # EnumUtil specialization
    file( APPEND "${WRAP_ENUM_DEST}" "template<> struct EnumUtil<${WRAP_ENUM_NAME}>\n" )
    file( APPEND "${WRAP_ENUM_DEST}" "{\n" )

    # fromString()
    file( APPEND "${WRAP_ENUM_DEST}" "    static ${WRAP_ENUM_NAME} fromString(const std::string& value)\n" )
    file( APPEND "${WRAP_ENUM_DEST}" "    {\n" )
    foreach( _val IN LISTS _vals )
        # remove trailing whitespace/comments
        string( REGEX REPLACE "//.*\$" "" _val "${_val}" )
        string( REPLACE " " "" _val "${_val}" )
        string( REPLACE "\t" "" _val "${_val}" )
        if( _val STREQUAL "" )
            continue()
        endif()
        string( REGEX REPLACE "=.+\$" "" _ename "${_val}" )
        if( _ename STREQUAL "" )
            continue()
        endif()
        if( _eval STREQUAL "" )
            continue()
        endif()
        # emit: if(value == "Value") return Enum::Value;
        file( APPEND "${WRAP_ENUM_DEST}" "        if(value == \"${_ename}\") return ${WRAP_ENUM_NAME}::${_ename};\n" )
    endforeach()
    file( APPEND "${WRAP_ENUM_DEST}" "        throw std::domain_error(\"Value '\" + value + \"' invalid for enum '${WRAP_ENUM_NAME}'\");\n" )
    file( APPEND "${WRAP_ENUM_DEST}" "    }\n" )

    # all()
    file( APPEND "${WRAP_ENUM_DEST}" "    static std::map<${WRAP_ENUM_NAME}, std::string> all()\n" )
    file( APPEND "${WRAP_ENUM_DEST}" "    {\n" )
    file( APPEND "${WRAP_ENUM_DEST}" "        return {\n" )
    foreach( _val IN LISTS _vals )
        # remove trailing whitespace/comments
        string( REGEX REPLACE "//.*\$" "" _val "${_val}" )
        string( REPLACE " " "" _val "${_val}" )
        string( REPLACE "\t" "" _val "${_val}" )
        if( _val STREQUAL "" )
            continue()
        endif()
        string( REGEX REPLACE "=.+\$" "" _ename "${_val}" )
        if( _ename STREQUAL "" )
            continue()
        endif()
        if( _eval STREQUAL "" )
            continue()
        endif()
        # emit: { enum, string },
        file( APPEND "${WRAP_ENUM_DEST}" "            {${WRAP_ENUM_NAME}::${_ename}, \"${_ename}\"},\n" )
    endforeach()
    file( APPEND "${WRAP_ENUM_DEST}" "        };\n" )
    file( APPEND "${WRAP_ENUM_DEST}" "    }\n" )

    file( APPEND "${WRAP_ENUM_DEST}" "};\n" )
    file( APPEND "${WRAP_ENUM_DEST}" "} // namespace ${WRAP_ENUM_NAMESPACE}\n" )
endfunction()
