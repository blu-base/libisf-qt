set( CMAKE_BUILD_TYPE debugfull )

if( CMAKE_COMPILER_IS_GNUCXX )
  set( CMAKE_CXX_FLAGS_DEBUGFULL "-O0 -g3 -fno-inline -Wall -Woverloaded-virtual -Wsign-compare -Wundef -fvisibility=default" )
endif( CMAKE_COMPILER_IS_GNUCXX )


set( TEST_DECODE_SOURCES
     main.cpp
   )

set( TEST_DECODE_MOC_HEADERS
#      testdecode.h
   )

set( TEST_DECODE_UI
     testdecode.ui
)

find_package( Qt4 REQUIRED )
include( ${QT_USE_FILE} )
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

qt4_wrap_ui( TEST_DECODE_UI_HEADERS ${TEST_DECODE_UI} )
qt4_wrap_cpp( TEST_DECODE_MOC_SOURCES ${TEST_DECODE_MOC_HEADERS} )

add_executable( test_decode
                ${TEST_DECODE_MOC_SOURCES}
                ${TEST_DECODE_SOURCES}
                ${TEST_DECODE_UI_HEADERS}
              )

target_link_libraries( test_decode ${QT_QTGUI_LIBRARY} isf-qt )
