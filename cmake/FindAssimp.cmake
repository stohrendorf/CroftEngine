include(FindPackageHandleStandardArgs)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(ASSIMP_ARCHITECTURE "64")
elseif()
	set(ASSIMP_ARCHITECTURE "32")
endif()

if(WIN32)
	set(ASSIMP_ROOT_DIR CACHE PATH "ASSIMP root directory")

	# Find path of each library
	find_path(Assimp_INCLUDE_DIRS
		NAMES
			assimp/anim.h
		HINTS
			${ASSIMP_ROOT_DIR}/include
	)

	if(MSVC12)
		set(ASSIMP_MSVC_VERSION "vc120")
	elseif(MSVC14)
		set(ASSIMP_MSVC_VERSION "vc140")
	endif()

	if(MSVC12 OR MSVC14)
		find_path(ASSIMP_LIBRARY_DIR
			NAMES
				assimp-${ASSIMP_MSVC_VERSION}-mt.lib
				assimp-${ASSIMP_MSVC_VERSION}-mtd.lib
			HINTS
				${ASSIMP_ROOT_DIR}/lib${ASSIMP_ARCHITECTURE}
				${ASSIMP_ROOT_DIR}/lib
		)

		find_library(ASSIMP_LIBRARY_RELEASE assimp-${ASSIMP_MSVC_VERSION}-mt.lib  PATHS ${ASSIMP_LIBRARY_DIR})
		find_library(ASSIMP_LIBRARY_DEBUG   assimp-${ASSIMP_MSVC_VERSION}-mtd.lib PATHS ${ASSIMP_LIBRARY_DIR})

		set(Assimp_LIBRARIES
			optimized ${ASSIMP_LIBRARY_RELEASE}
			debug     ${ASSIMP_LIBRARY_DEBUG}
		)

		FUNCTION(ASSIMP_COPY_BINARIES TargetDirectory)
			ADD_CUSTOM_TARGET(AssimpCopyBinaries
				COMMAND ${CMAKE_COMMAND} -E copy ${ASSIMP_ROOT_DIR}/bin${ASSIMP_ARCHITECTURE}/assimp-${ASSIMP_MSVC_VERSION}-mtd.dll 	${TargetDirectory}/Debug/assimp-${ASSIMP_MSVC_VERSION}-mtd.dll
				COMMAND ${CMAKE_COMMAND} -E copy ${ASSIMP_ROOT_DIR}/bin${ASSIMP_ARCHITECTURE}/assimp-${ASSIMP_MSVC_VERSION}-mt.dll 		${TargetDirectory}/Release/assimp-${ASSIMP_MSVC_VERSION}-mt.dll
				COMMENT "Copying Assimp binaries to '${TargetDirectory}'"
				VERBATIM
			)
		ENDFUNCTION()
	endif()
else()
	find_path(
		Assimp_INCLUDE_DIRS
		NAMES postprocess.h scene.h version.h config.h cimport.h
		PATHS /usr/local/include/
		      /usr/include
	)

	find_library(
		Assimp_LIBRARIES
		NAMES assimp
		PATHS /usr/local/lib/
	)
endif()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Assimp DEFAULT_MSG Assimp_INCLUDE_DIRS Assimp_LIBRARIES)
