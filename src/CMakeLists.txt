
include_directories(${QT_INCLUDES})

set ( libisf_sources
      isfdrawing.cpp
      tags.cpp
      multibytecoding.cpp
      compression/compression.cpp
      compression/gorilla.cpp
      compression/huffman.cpp
      compression/isfdata.cpp
    )

add_library( isf-qt SHARED  ${libisf_sources} )

target_link_libraries( isf-qt ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} )

set_target_properties( isf-qt PROPERTIES
                              VERSION ${LIBISFQT_VERSION}
                              SOVERSION ${LIBISFQT_VERSION} )

install( TARGETS isf-qt LIBRARY DESTINATION lib )