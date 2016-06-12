function(gen_enum name type source dest)
    if(EXISTS "${dest}" AND "${dest}" IS_NEWER_THAN "${source}")
        return()
    endif()
    message(STATUS "Generating enum include ${dest} from ${source}")
    file(STRINGS "${source}" _vals)
    file(WRITE "${dest}" "")
    # Write enum
    file(APPEND "${dest}" "enum class ${name} : ${type}\n")
    file(APPEND "${dest}" "{\n")
    foreach(_val IN LISTS _vals)
        string(REGEX REPLACE "\\s*//.*" "" _val "${_val}")
        string(REGEX REPLACE "=.+\$" "" _ename "${_val}")
        if("${_ename}" STREQUAL "")
            continue()
        endif()
        string(REGEX REPLACE "^.+=([0-9]+)" "\\1" _eval "${_val}")
        if("${_eval}" STREQUAL "")
            continue()
        endif()
        file(APPEND "${dest}" "    ${_ename} = ${_eval},\n")
    endforeach()
    file(APPEND "${dest}" "};\n\n")
    # Write toString
    file(APPEND "${dest}" "inline const char* toString(${name} val)\n")
    file(APPEND "${dest}" "{\n")
    file(APPEND "${dest}" "    switch(val)\n")
    file(APPEND "${dest}" "    {\n")
    foreach(_val IN LISTS _vals)
        string(REGEX REPLACE "\\s*//.*" "" _val "${_val}")
        string(REGEX REPLACE "=.+\$" "" _ename "${_val}")
        if("${_ename}" STREQUAL "")
            continue()
        endif()
        string(REGEX REPLACE "^.+=([0-9]+)" "\\1" _eval "${_val}")
        if("${_eval}" STREQUAL "")
            continue()
        endif()
        file(APPEND "${dest}" "        case ${name}::${_ename}: return \"${_ename}(${_eval})\";\n")
    endforeach()
    file(APPEND "${dest}" "        default: return nullptr;\n")
    file(APPEND "${dest}" "    }\n")
    file(APPEND "${dest}" "}\n")
endfunction()

gen_enum(AnimationId uint16_t ${CMAKE_CURRENT_SOURCE_DIR}/animationids.txt ${CMAKE_CURRENT_SOURCE_DIR}/loader/animationids_enum.h)
gen_enum(LaraStateId uint16_t ${CMAKE_CURRENT_SOURCE_DIR}/states.txt ${CMAKE_CURRENT_SOURCE_DIR}/loader/larastateids_enum.h)

set(OT_IRR_SRCS
    ot_irr.cpp
    loader/level.h
    loader/level.cpp
    loader/datatypes.h
    loader/datatypes.cpp
    loader/tr1level.h
    loader/tr1level.cpp
    loader/tr2level.h
    loader/tr2level.cpp
    loader/tr3level.h
    loader/tr3level.cpp
    loader/tr4level.h
    loader/tr4level.cpp
    loader/tr5level.h
    loader/tr5level.cpp
    loader/trcamerascenenodeanimator.h
    loader/trcamerascenenodeanimator.cpp
    loader/defaultanimdispatcher.h
    loader/defaultanimdispatcher.cpp
    loader/larastatehandler.h
    loader/larastatehandler.cpp
    loader/larastateid.h

    inputstate.h
    heightinfo.h
    heightinfo.cpp
    larastate.h
    larastate.cpp
    abstractstatehandler.h
    abstractstatehandler.cpp

    core/magic.h
)

add_executable(ot_irr
    ${OT_IRR_SRCS}
)

GROUP_FILES(${OT_IRR_SRCS})

if(NOT MSVC)
    target_compile_options(ot_irr PRIVATE -std=c++11 -Wall -Wextra -msse3)
endif()

target_link_libraries(
    ot_irr
    Boost
    Irrlicht
    SDL2
    ZLIB
)

target_include_directories(ot_irr PRIVATE . ../3rdparty/GSL/include)
