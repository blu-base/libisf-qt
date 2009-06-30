
include_directories(${QT_INCLUDES})

set ( libisf_sources
      isfdrawing.cpp
      format/types.cpp
      compression/huffman.cpp
      compression/gorilla.cpp
      compression/compression.cpp
    )
    
add_library( isf-qt SHARED  ${libisf_sources} )

target_link_libraries( isf-qt ${QT_QTCORE_LIBRARY} )

set_target_properties( isf-qt PROPERTIES
                              VERSION ${LIBISFQT_VERSION}
                              SOVERSION ${LIBISFQT_VERSION} )

install( TARGETS isf-qt LIBRARY DESTINATION lib )