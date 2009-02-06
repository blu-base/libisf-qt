
INCLUDE_DIRECTORIES( ${QT_INCLUDES} )


SET( LIBISF_SOURCES
     libisf.cpp
)

set( LIBISF_MOC_HEADERS
     libisf.h
)

# add some useful macros and variables
# (QT_USE_FILE is a variable defined by FIND_PACKAGE( Qt4 ) that contains a path to CMake script)
INCLUDE( ${QT_USE_FILE} )

# Run Qt's moc on the headers of QObject-derived classed
# QT4_WRAP_CPP( LIBISF_MOC_SOURCES ${LIBISF_MOC_HEADERS} )

ADD_LIBRARY( LibIsf SHARED
 ${LIBISF_SOURCES}
#  ${LIBISF_MOC_SOURCES}
)

target_link_libraries( LibIsf ${QT_QTCORE_LIBRARY} )

set_target_properties( LibIsf PROPERTIES
                              VERSION ${LIBISF_VERSION}
                              SOVERSION ${LIBISF_VERSION}
)



INSTALL( TARGETS LibIsf LIBRARY DESTINATION lib )
