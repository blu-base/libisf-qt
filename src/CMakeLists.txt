
#### Source files definitions ####

SET( ISFQT_SOURCES
     compression/compression.cpp
     compression/gorilla.cpp
     compression/huffman.cpp
     compression/isfdata.cpp
     isfdrawing.cpp
     multibytecoding.cpp
     tags.cpp
   )

SET( ISFQT_PUBLIC_HEADERS
     ../include/isfqt.h
     ../include/isfdrawing.h
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
         DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"     )
INSTALL( FILES ${ISFQT_PUBLIC_HEADERS}
         DESTINATION "${CMAKE_INSTALL_PREFIX}/include" )
