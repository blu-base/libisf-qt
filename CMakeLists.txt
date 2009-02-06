PROJECT( libisf )
CMAKE_MINIMUM_REQUIRED( VERSION 2.4.5 )


# Version number
SET( LIBISF_VERSION "0.1" )

# Define compiler flags
# Tune the debug build with even more flags
IF( CMAKE_COMPILER_IS_GNUCXX )
  # skipped: -ansi -pendantic -Wfatal-errors -Wold-style-cast -Wconversion
  SET( CMAKE_CXX_FLAGS_DEBUG     "-O0 -g -Wall" )
  SET( CMAKE_CXX_FLAGS_DEBUGFULL "-O0 -g3 -fno-inline -Wall -Woverloaded-virtual -Wsign-compare -Wundef" )
ENDIF( CMAKE_COMPILER_IS_GNUCXX )


# Define the default build type
# Possible values:
# - none
# - release
# - debug
# - debugfull       (even fewer optimisations)
# - relwithdebinfo  (release with debug info)
# - minsizerel      (minsize release)
# Uncomment the next line to always force building in full debug mode
SET( CMAKE_BUILD_TYPE debugfull )

# Enable debug output when compiling a debug build
IF( CMAKE_BUILD_TYPE STREQUAL debugfull OR LIBISF_DEBUG_OUTPUT EQUAL 1 )
  SET( LIBISF_ENABLE_DEBUG_OUTPUT "1" )
ELSE( CMAKE_BUILD_TYPE STREQUAL debugfull OR LIBISF_DEBUG_OUTPUT EQUAL 1 )
  SET( LIBISF_ENABLE_DEBUG_OUTPUT "0" )
ENDIF( CMAKE_BUILD_TYPE STREQUAL debugfull OR LIBISF_DEBUG_OUTPUT EQUAL 1 )

# Set a default installation path
IF( NOT CMAKE_INSTALL_PREFIX )
  SET( CMAKE_INSTALL_PREFIX "/usr" )
ENDIF( NOT CMAKE_INSTALL_PREFIX )

# Make the code aware of the CMake flags
CONFIGURE_FILE( ${CMAKE_CURRENT_SOURCE_DIR}/cmake/libisfconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/libisfconfig.h )
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )

# We only require Qt
find_package( Qt4 REQUIRED )

ADD_SUBDIRECTORY( src )
