
#### Source files definitions ####

SET( ISFQT_SOURCES
     data/compression.cpp
     data/gorilla.cpp
     data/huffman.cpp
     data/datasource.cpp
     data/multibytecoding.cpp
     isfqtdrawing.cpp
     tagsparser.cpp
     tagswriter.cpp
     isfqt.cpp
   )

SET( ISFQT_PUBLIC_HEADERS
     ../include/isfqt.h
     ../include/isfqtdrawing.h
     ../include/IsfQt
     ../include/IsfQtDrawing
   )


#### Compilation ####

INCLUDE_DIRECTORIES( ${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} )

ADD_LIBRARY( isf-qt SHARED ${ISFQT_SOURCES} )
TARGET_LINK_LIBRARIES( isf-qt ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} )

SET_TARGET_PROPERTIES( isf-qt PROPERTIES
                              VERSION    ${ISFQT_VERSION}
                              SOVERSION  ${ISFQT_VERSION} )


#### Installation ####

INSTALL( TARGETS isf-qt LIBRARY
         DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"            )
INSTALL( FILES ${ISFQT_PUBLIC_HEADERS}
         DESTINATION "${CMAKE_INSTALL_PREFIX}/include/isf-qt" )
