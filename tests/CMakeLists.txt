
INCLUDE_DIRECTORIES( ${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} )
SET( CMAKE_EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

MACRO( add_multiple_tests )
  FOREACH( test ${ARGN} )
    QT4_AUTOMOC( test_${test}.cpp )
    ADD_EXECUTABLE( test_${test} test_${test}.cpp )
    ADD_TEST( libisf-${test} test_${test} )
    TARGET_LINK_LIBRARIES( test_${test} ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} isf-qt )
  ENDFOREACH( test )
ENDMACRO( add_multiple_tests )

ADD_MULTIPLE_TESTS(
  isfdrawing
  multibyte_coding
  algorithms
)


# Compile the ISF decoder test
ADD_SUBDIRECTORY( decode )
