PROJECT( isf-qt )
CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )


#### Main switches ####

# Bundled mode: you can add Isf-Qt as a subdirectory
# of your CMake-based prjoect easily. Just use:
# SET( USE_BUNDLED_LIBRARIES ISFQT )
# and Isf-Qt will be built and statically linked in your
# application, without installing anything
IF( ISFQT_IS_BUNDLED OR USE_BUNDLED_LIBRARIES MATCHES "ISFQT" )
  SET( ISFQT_IS_BUNDLED TRUE )
  MESSAGE( "-- Will build Isf-Qt in bundled mode." )
ELSE()
  SET( ISFQT_IS_BUNDLED FALSE )
ENDIF()

# Only set the build type if we're not being bundled: if we are, we'll
# use the build type specified by the parent project
IF( NOT ISFQT_IS_BUNDLED )

  # Define the default build type
  # Possible values:
  # - none
  # - release
  # - debug
  # - debugfull       (even fewer optimisations)
  # - relwithdebinfo  (release with debug info)
  # - minsizerel      (minimum size release)
  # Uncomment the next line to force building in full debug mode
  SET( CMAKE_BUILD_TYPE debugfull )
ENDIF()

# Define the library version number
SET( ISFQT_VERSION "0.9" )


#### Inclusions ####

FIND_PACKAGE( Qt4 REQUIRED )

OPTION( WANT_GIF "Enable support for reading and writing Fortified-GIF images" ON )
IF( WANT_GIF )
  MESSAGE( "-- Looking for GifLib" )

  # Apparently, the KDE CMake macros unset a check which is required by giflib.
  IF( KDE4_FOUND )
    INCLUDE( "cmake/giflib_hack.cmake" )
  ENDIF( KDE4_FOUND )

  FIND_PACKAGE( GIF QUIET )

  # At this point, we can be relatively sure that if it's there, it has been found
  IF( NOT GIF_FOUND )
    MESSAGE( "-- Looking for GifLib - not found!" )
    MESSAGE( FATAL_ERROR
             "Support to read and write Fortified-GIF images is missing.\n"
             "  * You will need to install both the library and development packages of the GIF Library, giflib.\n"
             "  * Usually the package names for this library are:\n"
             "      on Ubuntu:   'libgif4' and 'libgif-dev'\n"
             "      on openSUSE: 'giflib'  and 'giflib-devel'\n"
             "  * The project homepage is http://sourceforge.net/projects/giflib/\n" )
  ELSE()
    MESSAGE( "-- Looking for GifLib - found" )
  ENDIF()
ELSE( WANT_GIF )
  MESSAGE( "-- GifLib support disabled" )
ENDIF( WANT_GIF )

# Set the variable for isfqtconfig.h.in
IF( WANT_GIF )
  SET( GIF_ENABLED 1 )
ELSE()
  SET( GIF_ENABLED 0 )
ENDIF()


#### Compilation ####

IF( CMAKE_COMPILER_IS_GNUCXX )
  SET( CMAKE_CXX_FLAGS_DEBUGFULL
       "-O0 -g3 -fno-inline -Wall -Woverloaded-virtual -Wsign-compare -Wundef -fvisibility=default" )
ENDIF( CMAKE_COMPILER_IS_GNUCXX )

IF( NOT ISFQT_DEBUG_OUTPUT STREQUAL "1" AND NOT ISFQT_DEBUG_OUTPUT STREQUAL "0" )
  IF( CMAKE_BUILD_TYPE MATCHES debug )
    SET( ISFQT_DEBUG_OUTPUT 1 )
  ELSE()
    SET( ISFQT_DEBUG_OUTPUT 0 )
  ENDIF()
ENDIF()



INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} src include )

CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/cmake/isfqtconfig.h.in
                ${CMAKE_CURRENT_BINARY_DIR}/isfqtconfig.h )

ADD_SUBDIRECTORY( src   )

IF( NOT ISFQT_IS_BUNDLED )
  ENABLE_TESTING()
  ADD_SUBDIRECTORY( tests )
ENDIF()


#### Project CMake configuration files creation ####

# Define the shortcuts for the installation directories
SET( INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include/isf-qt"
                         CACHE STRING "Directory where the include files will be installed" )
SET( LIB_INSTALL_DIR     "${CMAKE_INSTALL_PREFIX}/lib"
                         CACHE STRING "Directory where the lib will be installed" )
SET( SHARED_INSTALL_DIR  "${CMAKE_INSTALL_PREFIX}/share"
                         CACHE STRING "Directory where the shared data will be installed" )

# Prepare thej IsfQtConfig.cmake file, which allows other projects to find us
CONFIGURE_FILE( cmake/IsfQtConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/IsfQtConfig.cmake" @ONLY )


#### Installation ####

# Do not install the resources if we're bundling it into an application
IF( NOT ISFQT_IS_BUNDLED )
  INSTALL( FILES ${CMAKE_CURRENT_BINARY_DIR}/IsfQtConfig.cmake
          DESTINATION ${LIB_INSTALL_DIR}/isfqt )
  INSTALL( FILES cmake/FindIsfQt.cmake
          DESTINATION ${CMAKE_ROOT}/Modules )
ENDIF()

