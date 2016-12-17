include( FindPackageHandleStandardArgs )

if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set( ASSIMP_ARCHITECTURE "64" )
elseif()
    set( ASSIMP_ARCHITECTURE "32" )
endif()

set( ASSIMP_ROOT_DIR CACHE PATH "ASSIMP root directory" )

find_path( Assimp_INCLUDE_DIRS
           NAMES
           assimp/anim.h
           HINTS
           ${ASSIMP_ROOT_DIR}/include
           /usr/include
           /usr/local/include
           )

if( MSVC12 OR MSVC14 )
    if( MSVC12 )
        set( ASSIMP_MSVC_VERSION "vc120" )
    elseif( MSVC14 )
        set( ASSIMP_MSVC_VERSION "vc140" )
    endif()

    find_path( ASSIMP_LIBRARY_DIR
               NAMES
               assimp-${ASSIMP_MSVC_VERSION}-mt.lib
               assimp-${ASSIMP_MSVC_VERSION}-mtd.lib
               HINTS
               ${ASSIMP_ROOT_DIR}/lib${ASSIMP_ARCHITECTURE}
               ${ASSIMP_ROOT_DIR}/lib
               )

    find_library( ASSIMP_LIBRARY_RELEASE assimp-${ASSIMP_MSVC_VERSION}-mt.lib PATHS ${ASSIMP_LIBRARY_DIR} )
    find_library( ASSIMP_LIBRARY_DEBUG assimp-${ASSIMP_MSVC_VERSION}-mtd.lib PATHS ${ASSIMP_LIBRARY_DIR} )

    if(ASSIMP_LIBRARY_RELEASE AND ASSIMP_LIBRARY_DEBUG)
      set(Assimp_LIBRARIES optimized ${ASSIMP_LIBRARY_RELEASE} debug ${ASSIMP_LIBRARY_DEBUG})
    elseif(ASSIMP_LIBRARY_RELEASE)
      set(Assimp_LIBRARIES ${Assimp_LIBRARIES} ${ASSIMP_LIBRARY_RELEASE})
    endif()
else()
    find_library(
            Assimp_LIBRARIES
            NAMES assimp
            PATHS
            /usr/lib/
            /usr/lib${ASSIMP_ARCHITECTURE}/
            /usr/local/lib/
            /usr/local/lib${ASSIMP_ARCHITECTURE}/
            ${ASSIMP_ROOT_DIR}/usr/local/lib/
            ${ASSIMP_ROOT_DIR}/usr/local/lib${ASSIMP_ARCHITECTURE}/
    )
endif()

FIND_PACKAGE_HANDLE_STANDARD_ARGS( Assimp DEFAULT_MSG Assimp_INCLUDE_DIRS Assimp_LIBRARIES )
