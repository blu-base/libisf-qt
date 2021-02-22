set( CMAKE_BUILD_TYPE debugfull )

#if( CMAKE_COMPILER_IS_GNUCXX )
#  set( CMAKE_CXX_FLAGS_DEBUGFULL "-O0 -g3 -fno-inline -Wall -Woverloaded-virtual -Wsign-compare -Wundef -fvisibility=default" )
#endif( CMAKE_COMPILER_IS_GNUCXX )


set( TEST_DECODE_SOURCES
     main.cpp
   )

set( TEST_DECODE_MOC_HEADERS
#      testdecode.h
   )

set( TEST_DECODE_UI
     testdecode.ui
)

qt5_wrap_ui( TEST_DECODE_UI_HEADERS ${TEST_DECODE_UI} )
qt5_wrap_cpp( TEST_DECODE_MOC_SOURCES ${TEST_DECODE_MOC_HEADERS} )

add_executable( test_decode
                ${TEST_DECODE_MOC_SOURCES}
                ${TEST_DECODE_SOURCES}
                ${TEST_DECODE_UI_HEADERS}
              )

target_include_directories( test_decode PRIVATE ${CMAKE_CURRENT_BINARY_DIR} )

target_link_libraries( test_decode PRIVATE Qt5::Gui isf-qt )
