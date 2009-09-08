
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
   )

SET( ISFQT_PUBLIC_HEADERS
     ../include/isfqt.h
     ../include/isfqtdrawing.h
     ../include/isfinkcanvas.h
     ../include/IsfQt
     ../include/IsfQtDrawing
     ../include/IsfInkCanvas
   )

SET( ISFQT_RCCS 
    resources.qrc
   )

#### Compilation ####

QT4_WRAP_CPP( MOC_SRCS ../include/isfinkcanvas.h )

QT4_ADD_RESOURCES( ISFQT_RCC_SRCS ${ISFQT_RCCS} )

INCLUDE_DIRECTORIES( ${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} )

IF ( NOT IS_BUNDLED_BUILD )
  ADD_LIBRARY( isf-qt SHARED ${ISFQT_SOURCES} ${MOC_SRCS} ${ISFQT_RCC_SRCS} )
ELSE ( NOT IS_BUNDLED_BUILD )
  ADD_LIBRARY( isf-qt STATIC ${ISFQT_SOURCES} ${MOC_SRCS} ${ISFQT_RCC_SRCS} )
ENDIF ( NOT IS_BUNDLED_BUILD )

TARGET_LINK_LIBRARIES( isf-qt ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} )

SET_TARGET_PROPERTIES( isf-qt PROPERTIES
                              VERSION    ${ISFQT_VERSION}
                              SOVERSION  ${ISFQT_VERSION} )

#### Installation ####
IF ( NOT IS_BUNDLED_BUILD )
  INSTALL( TARGETS isf-qt LIBRARY
          DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"            )
  INSTALL( FILES ${ISFQT_PUBLIC_HEADERS}
          DESTINATION "${CMAKE_INSTALL_PREFIX}/include/isf-qt" )
ENDIF( NOT IS_BUNDLED_BUILD )
