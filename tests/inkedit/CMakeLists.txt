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

set( TEST_INKEDIT_UI 
    testinkedit.ui
   )

qt5_wrap_cpp( TEST_INKEDIT_MOC_SOURCES ${TEST_INKEDIT_MOC_HEADERS} )
qt5_wrap_ui( TEST_INKEDIT_UI_HEADERS ${TEST_INKEDIT_UI} )

add_executable( test_inkedit
                ${TEST_INKEDIT_MOC_SOURCES}
                ${TEST_INKEDIT_SOURCES}
                ${TEST_INKEDIT_UI_HEADERS}
              )
target_include_directories( test_inkedit PRIVATE ${CMAKE_CURRENT_BINARY_DIR} )

target_link_libraries( test_inkedit Qt5::Core Qt5::Gui Qt5::Widgets isf-qt )
