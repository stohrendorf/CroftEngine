function( wrap_enum )
    cmake_parse_arguments(
            WRAP_ENUM
            "" # no options
            "NAMESPACE;NAME;TYPE;SOURCE;DEST;DEST_IMPL"
            "WITH_INCLUDES"
            ${ARGN}
    )

    if( WRAP_ENUM_UNPARSED_ARGUMENTS )
        message( FATAL_ERROR "Unexpected ${WRAP_ENUM_UNPARSED_ARGUMENTS}" )
    endif()
    if( WRAP_ENUM_KEYWORDS_MISSING_VALUES )
        message( FATAL_ERROR "Missing ${WRAP_ENUM_KEYWORDS_MISSING_VALUES}" )
    endif()

    if(
            EXISTS "${WRAP_ENUM_DEST}" AND "${WRAP_ENUM_DEST}" IS_NEWER_THAN "${WRAP_ENUM_SOURCE}"
            AND EXISTS "${WRAP_ENUM_DEST_IMPL}" AND "${WRAP_ENUM_DEST_IMPL}" IS_NEWER_THAN "${WRAP_ENUM_SOURCE}"
    )
        return()
    endif()

    message( STATUS "Generating ${WRAP_ENUM_DEST} and ${WRAP_ENUM_DEST_IMPL} from ${WRAP_ENUM_SOURCE}" )
    file( STRINGS "${WRAP_ENUM_SOURCE}" _vals )

    set( _wrap_enum_tmp ${CMAKE_BINARY_DIR}/wrap-enum.tmp )
    file( WRITE "${_wrap_enum_tmp}" "" )
    # headers
    file( APPEND "${_wrap_enum_tmp}" "#pragma once\n" )
    file( APPEND "${_wrap_enum_tmp}" "\n" )
    file( APPEND "${_wrap_enum_tmp}" "#include <cstdint>\n" )
    file( APPEND "${_wrap_enum_tmp}" "#include <gsl/gsl-lite.hpp>\n" )
    file( APPEND "${_wrap_enum_tmp}" "#include <map>\n" )
    file( APPEND "${_wrap_enum_tmp}" "#include <string>\n" )
    foreach( _include IN LISTS WRAP_ENUM_WITH_INCLUDES )
        if( NOT ${_include} MATCHES "^<" )
            set( _include "\"${include}\"" )
        endif()
        file( APPEND "${_wrap_enum_tmp}" "#include ${_include}\n" )
    endforeach()
    file( APPEND "${_wrap_enum_tmp}" "\n" )

    # Write enum
    file( APPEND "${_wrap_enum_tmp}" "namespace ${WRAP_ENUM_NAMESPACE}\n" )
    file( APPEND "${_wrap_enum_tmp}" "{\n" )
    file( APPEND "${_wrap_enum_tmp}" "enum class ${WRAP_ENUM_NAME} : ${WRAP_ENUM_TYPE}\n" )
    file( APPEND "${_wrap_enum_tmp}" "{\n" )
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
            file( APPEND "${_wrap_enum_tmp}" "    ${_ename} = ${_eval},\n" )
        else()
            # emit: Value,
            file( APPEND "${_wrap_enum_tmp}" "    ${_ename},\n" )
        endif()
    endforeach()
    file( APPEND "${_wrap_enum_tmp}" "};\n\n" )

    # toString function
    file( APPEND "${_wrap_enum_tmp}" "extern gsl::czstring toString(${WRAP_ENUM_NAME} val);\n\n" )

    # EnumUtil forward declaration
    file( APPEND "${_wrap_enum_tmp}" "template<typename T> struct EnumUtil;\n\n" )
    # EnumUtil specialization
    file( APPEND "${_wrap_enum_tmp}" "template<> struct EnumUtil<${WRAP_ENUM_NAME}>\n" )
    file( APPEND "${_wrap_enum_tmp}" "{\n" )

    # fromString()
    file( APPEND "${_wrap_enum_tmp}" "    static ${WRAP_ENUM_NAME} fromString(const std::string& value);\n" )

    # all()
    file( APPEND "${_wrap_enum_tmp}" "    static std::map<${WRAP_ENUM_NAME}, std::string> all();\n" )

    # name()
    file( APPEND "${_wrap_enum_tmp}" "    static const char* name()\n" )
    file( APPEND "${_wrap_enum_tmp}" "    {\n" )
    file( APPEND "${_wrap_enum_tmp}" "        return \"${WRAP_ENUM_NAME}\";\n" )
    file( APPEND "${_wrap_enum_tmp}" "    }\n" )

    file( APPEND "${_wrap_enum_tmp}" "};\n" )
    file( APPEND "${_wrap_enum_tmp}" "} // namespace ${WRAP_ENUM_NAMESPACE}\n" )
    file( RENAME ${_wrap_enum_tmp} ${WRAP_ENUM_DEST} )


    get_filename_component( _include_name ${WRAP_ENUM_DEST} NAME )
    file( WRITE "${_wrap_enum_tmp}" "" )
    # headers
    file( APPEND "${_wrap_enum_tmp}" "#include \"${_include_name}\"\n" )
    file( APPEND "${_wrap_enum_tmp}" "\n" )
    file( APPEND "${_wrap_enum_tmp}" "#include <cstdint>\n" )
    file( APPEND "${_wrap_enum_tmp}" "#include <gsl/gsl-lite.hpp>\n" )
    file( APPEND "${_wrap_enum_tmp}" "#include <map>\n" )
    file( APPEND "${_wrap_enum_tmp}" "#include <stdexcept>\n" )
    file( APPEND "${_wrap_enum_tmp}" "#include <string>\n" )
    file( APPEND "${_wrap_enum_tmp}" "\n" )

    file( APPEND "${_wrap_enum_tmp}" "namespace ${WRAP_ENUM_NAMESPACE}\n" )
    file( APPEND "${_wrap_enum_tmp}" "{\n" )

    # toString function
    file( APPEND "${_wrap_enum_tmp}" "gsl::czstring toString(${WRAP_ENUM_NAME} val)\n" )
    file( APPEND "${_wrap_enum_tmp}" "{\n" )
    file( APPEND "${_wrap_enum_tmp}" "    switch(val)\n" )
    file( APPEND "${_wrap_enum_tmp}" "    {\n" )
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
        file( APPEND "${_wrap_enum_tmp}" "        case ${WRAP_ENUM_NAME}::${_ename}: return \"${_ename}\";\n" )
    endforeach()
    file( APPEND "${_wrap_enum_tmp}" "        default: return nullptr;\n" )
    file( APPEND "${_wrap_enum_tmp}" "    }\n" )
    file( APPEND "${_wrap_enum_tmp}" "}\n\n" )

    # fromString()
    file( APPEND "${_wrap_enum_tmp}" "${WRAP_ENUM_NAME} EnumUtil<${WRAP_ENUM_NAME}>::fromString(const std::string& value)\n" )
    file( APPEND "${_wrap_enum_tmp}" "{\n" )
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
        file( APPEND "${_wrap_enum_tmp}" "    if(value == \"${_ename}\") return ${WRAP_ENUM_NAME}::${_ename};\n" )
    endforeach()
    file( APPEND "${_wrap_enum_tmp}" "    throw std::domain_error(\"Value '\" + value + \"' invalid for enum '${WRAP_ENUM_NAME}'\");\n" )
    file( APPEND "${_wrap_enum_tmp}" "}\n\n" )

    # all()
    file( APPEND "${_wrap_enum_tmp}" "std::map<${WRAP_ENUM_NAME}, std::string> EnumUtil<${WRAP_ENUM_NAME}>::all()\n" )
    file( APPEND "${_wrap_enum_tmp}" "{\n" )
    file( APPEND "${_wrap_enum_tmp}" "    return {\n" )
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
        file( APPEND "${_wrap_enum_tmp}" "        {${WRAP_ENUM_NAME}::${_ename}, \"${_ename}\"},\n" )
    endforeach()
    file( APPEND "${_wrap_enum_tmp}" "    };\n" )
    file( APPEND "${_wrap_enum_tmp}" "}\n" )

    file( APPEND "${_wrap_enum_tmp}" "} // namespace ${WRAP_ENUM_NAMESPACE}\n" )
    file( RENAME ${_wrap_enum_tmp} ${WRAP_ENUM_DEST_IMPL} )
endfunction()
