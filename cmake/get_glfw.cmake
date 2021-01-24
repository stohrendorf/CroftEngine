if( TARGET glfw )
    return()
endif()

find_package( glfw3 3.3 REQUIRED )
target_compile_definitions( glfw INTERFACE -DGLFW_INCLUDE_NONE )
