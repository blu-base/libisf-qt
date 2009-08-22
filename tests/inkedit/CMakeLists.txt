set( CMAKE_BUILD_TYPE debugfull )

if( CMAKE_COMPILER_IS_GNUCXX )
  set( CMAKE_CXX_FLAGS_DEBUGFULL "-O0 -g3 -fno-inline -Wall -Woverloaded-virtual -Wsign-compare -Wundef -fvisibility=default" )
endif( CMAKE_COMPILER_IS_GNUCXX )


set( TEST_INKEDIT_SOURCES
     main.cpp
   )

set( TEST_INKEDIT_MOC_HEADERS
    main.h
   )


find_package( Qt4 REQUIRED )
include( ${QT_USE_FILE} )
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

qt4_wrap_cpp( TEST_INKEDIT_MOC_SOURCES ${TEST_INKEDIT_MOC_HEADERS} )

add_executable( test_inkedit
                ${TEST_INKEDIT_MOC_SOURCES}
                ${TEST_INKEDIT_SOURCES}
              )

target_link_libraries( test_inkedit ${QT_QTGUI_LIBRARY} isf-qt )
