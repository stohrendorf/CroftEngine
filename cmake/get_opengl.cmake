if( TARGET OpenGL )
    return()
endif()

find_package( OpenGL REQUIRED )
add_library( OpenGL INTERFACE )
target_include_directories( OpenGL SYSTEM BEFORE INTERFACE ${OPENGL_INCLUDE_DIR} )
target_link_libraries( OpenGL INTERFACE ${OPENGL_LIBRARIES} )
