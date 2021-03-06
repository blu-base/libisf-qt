
#### Source files definitions ####

SET( ISFQT_SOURCES
     data/algorithms/bitpacking.cpp
     data/algorithms/bitpacking_byte.cpp
     data/algorithms/huffman.cpp
     data/algorithms/deltatransform.cpp
     data/compression.cpp
     data/datasource.cpp
     data/multibytecoding.cpp
     isfqtdrawing.cpp
     tagsparser.cpp
     tagswriter.cpp
     isfqt.cpp
     isfinkcanvas.cpp
     isfqtstroke.cpp
   )

SET( ISFQT_PUBLIC_HEADERS
     ../include/isfqt.h
     ../include/isfqtdrawing.h
     ../include/isfinkcanvas.h
     ../include/isfqtstroke.h
     ../include/IsfQtDrawing
     ../include/IsfInkCanvas
     ../include/IsfQtStroke
     ../include/IsfQt
   )

SET( ISFQT_RCCS
     ../data/isfqtresources.qrc
   )




#### Compilation ####

QT5_WRAP_CPP( MOC_SRCS ../include/isfinkcanvas.h )

QT5_ADD_RESOURCES( ISFQT_RCC_SRCS ${ISFQT_RCCS} )

INCLUDE_DIRECTORIES( ${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} )

# Do not install the library if we're bundling it into an application
IF( ISFQT_IS_BUNDLED )
  ADD_LIBRARY( isf-qt STATIC ${ISFQT_SOURCES} ${MOC_SRCS} ${ISFQT_RCC_SRCS} )
ELSE()
  ADD_LIBRARY( isf-qt SHARED ${ISFQT_SOURCES} ${MOC_SRCS} ${ISFQT_RCC_SRCS} )
ENDIF()

target_link_libraries( isf-qt
  PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
  )

if( GIF_FOUND )
  target_link_libraries( isf-qt PRIVATE gif )
endif()

SET_TARGET_PROPERTIES( isf-qt PROPERTIES
                              VERSION    ${ISFQT_VERSION}
                              SOVERSION  ${ISFQT_VERSION} )

#### Installation ####

# Do not install the library if we're bundling it in KMess
IF( NOT ISFQT_IS_BUNDLED )
  INSTALL( TARGETS isf-qt LIBRARY
          DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"            )
  INSTALL( FILES ${ISFQT_PUBLIC_HEADERS}
          DESTINATION "${CMAKE_INSTALL_PREFIX}/include/isf-qt" )
ENDIF()
