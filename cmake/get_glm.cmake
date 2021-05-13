if( TARGET glm::glm )
    return()
endif()

find_package( glm CONFIG REQUIRED )

if( NOT TARGET glm )
    add_library( glm INTERFACE IMPORTED )
    target_include_directories( glm SYSTEM BEFORE INTERFACE ${GLM_INCLUDE_DIR} )
endif()
if( NOT TARGET glm::glm )
    add_library( glm::glm INTERFACE IMPORTED )
    target_link_libraries( glm::glm INTERFACE glm )
endif()

target_compile_definitions( glm::glm INTERFACE -DGLM_ENABLE_EXPERIMENTAL -DGLM_FORCE_RADIANS -DGLM_FORCE_CXX${CMAKE_CXX_STANDARD} )
