if( TARGET LibArchive::LibArchive )
    return()
endif()

find_package( LibArchive REQUIRED )

if( NOT TARGET LibArchive::LibArchive )
    add_library( LibArchive::LibArchive INTERFACE IMPORTED )
    target_include_directories( LibArchive::LibArchive INTERFACE ${LibArchive_INCLUDE_DIRS} )
    target_link_libraries( LibArchive::LibArchive INTERFACE ${LibArchive_LIBRARIES} )
endif()
