
INCLUDE_DIRECTORIES( ${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} )
SET( CMAKE_EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

find_package(Qt5Test REQUIRED)

MACRO( add_multiple_tests )
  FOREACH( test ${ARGN} )
    ADD_EXECUTABLE( test_${test} test_${test}.cpp )
    ADD_TEST( libisf-${test} test_${test} )
    TARGET_LINK_LIBRARIES( test_${test} Qt5::Core Qt5::Test isf-qt )
    set_property(TARGET test_${test} PROPERTY AUTOMOC ON)
  ENDFOREACH( test )
ENDMACRO( add_multiple_tests )

ADD_MULTIPLE_TESTS(
  isfdrawing
  multibyte_coding
  algorithms
  png_fortification
)


# Compile the other testing applications
ADD_SUBDIRECTORY( decode )
ADD_SUBDIRECTORY( inkedit )
