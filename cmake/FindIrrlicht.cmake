# - Try to find Irrlicht
# Once done this will define
#
#  IRRLICHT_FOUND - system has Irrlicht
#  IRRLICHT_INCLUDE_DIRS - the Irrlicht include directory
#  IRRLICHT_LIBRARIES - Link these to use Irrlicht
#  IRRLICHT_DEFINITIONS - Compiler switches required for using Irrlicht
#
#  Copyright (c) 2006 Andreas Schneider <mail@cynapses.org>
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#


if(NOT IRRLICHT_FOUND)
  find_path(IRRLICHT_INCLUDE_DIR
    NAMES
      irrlicht.h
    PATH_SUFFIXES
      include
      include/irrlicht
    PATHS
      ${IRRLICHT_ROOT}
      /usr
      /usr/local
      /sw
  )

  find_library(IRRLICHT_LIBRARY
    NAMES
     	Irrlicht
    PATH_SUFFIXES
      include
      include/irrlicht
    PATHS
      ${IRRLICHT_ROOT}
      /usr
      /usr/local
      /sw
  )

  set(IRRLICHT_INCLUDE_DIRS
    ${IRRLICHT_INCLUDE_DIR}
  )

  set(IRRLICHT_LIBRARIES
    ${IRRLICHT_LIBRARY}
  )

  # show the IRRLICHT_INCLUDE_DIRS and IRRLICHT_LIBRARIES variables only in the advanced view
  mark_as_advanced(IRRLICHT_INCLUDE_DIRS IRRLICHT_LIBRARIES)
endif()

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Irrlicht DEFAULT_MSG
    IRRLICHT_LIBRARIES IRRLICHT_INCLUDE_DIRS)

add_library(Irrlicht INTERFACE)
target_link_libraries(Irrlicht INTERFACE ${IRRLICHT_LIBRARIES})
target_include_directories(Irrlicht INTERFACE ${IRRLICHT_INCLUDE_DIRS})
