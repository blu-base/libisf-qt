# Find LibISF-Qt
# Find the Ink Serialized Format library and includes
# This module defines
#  ISFQT_FOUND       - If false, do not try to use ISF-Qt
#  ISFQT_INCLUDE_DIR - where to find isfqtdrawing.h and friends
#  ISFQT_LIBRARIES   - the libraries needed to use ISF-Qt
#  ISFQT_LIBRARY     - Where to find the ISF-Qt library (generally not used)

FIND_PATH( ISFQT_INCLUDE_DIR isfqtdrawing.h /usr/include/isf-qt /usr/local/include/isf-qt )
FIND_LIBRARY( ISFQT_LIBRARY NAMES isf-qt PATH /usr/lib /usr/local/lib )

# handle the QUIETLY and REQUIRED arguments and set ISFQT_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( ISFQT DEFAULT_MSG ISFQT_LIBRARY ISFQT_INCLUDE_DIR )

IF( ISFQT_FOUND )
  SET( ISFQT_LIBRARIES ${ISFQT_LIBRARY} )
ENDIF( ISFQT_FOUND )
