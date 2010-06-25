
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
   )

SET( ISFQT_LIBS
     ${QT_QTCORE_LIBRARY}
     ${QT_QTGUI_LIBRARY}
   )

SET( ISFQT_RCCS
     ../data/isfqtresources.qrc
   )

IF( GIF_FOUND )
  INCLUDE_DIRECTORIES( ${GIF_INCLUDE_DIR} )
  SET( ISFQT_LIBS ${ISFQT_LIBS} ${GIF_LIBRARIES} )
ENDIF( GIF_FOUND )


#### Compilation ####

QT4_WRAP_CPP( MOC_SRCS ../include/isfinkcanvas.h )

QT4_ADD_RESOURCES( ISFQT_RCC_SRCS ${ISFQT_RCCS} )

INCLUDE_DIRECTORIES( ${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} )

# Do not install the library if we're bundling it in KMess
IF( NOT USE_BUNDLED_LIBRARIES MATCHES "ISFQT" )
  ADD_LIBRARY( isf-qt SHARED ${ISFQT_SOURCES} ${MOC_SRCS} ${ISFQT_RCC_SRCS} )
ELSE()
  ADD_LIBRARY( isf-qt STATIC ${ISFQT_SOURCES} ${MOC_SRCS} ${ISFQT_RCC_SRCS} )
ENDIF()

TARGET_LINK_LIBRARIES( isf-qt ${ISFQT_LIBS} )

SET_TARGET_PROPERTIES( isf-qt PROPERTIES
                              VERSION    ${ISFQT_VERSION}
                              SOVERSION  ${ISFQT_VERSION} )

#### Installation ####

# Do not install the library if we're bundling it in KMess
IF( NOT USE_BUNDLED_LIBRARIES MATCHES "ISFQT" )
  INSTALL( TARGETS isf-qt LIBRARY
          DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"            )
  INSTALL( FILES ${ISFQT_PUBLIC_HEADERS}
          DESTINATION "${CMAKE_INSTALL_PREFIX}/include/isf-qt" )
ENDIF()
