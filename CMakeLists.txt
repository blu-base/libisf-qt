project(libisf)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)

enable_testing()

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)

set ( LIBISFQT_VERSION "0.1" )

set ( CMAKE_BUILD_TYPE debug )

add_subdirectory(src)
add_subdirectory(tests)
