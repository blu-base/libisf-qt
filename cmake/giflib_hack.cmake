
# This file is included here because the standard FindGIF.cmake file
# requires the CHECK_STRUCT_MEMBER macro. It wasn't on my PC, at least, so
# I include a copy of the macro here for giflib to use.

MACRO (CHECK_STRUCT_MEMBER _STRUCT _MEMBER _HEADER _RESULT)
   SET(_INCLUDE_FILES)
   FOREACH (it ${_HEADER})
      SET(_INCLUDE_FILES "${_INCLUDE_FILES}#include <${it}>\n")
   ENDFOREACH (it)

   SET(_CHECK_STRUCT_MEMBER_SOURCE_CODE "
${_INCLUDE_FILES}
int main()
{
   ${_STRUCT}* tmp;
   tmp->${_MEMBER};
  return 0;
}
")

   CHECK_CXX_SOURCE_COMPILES("${_CHECK_STRUCT_MEMBER_SOURCE_CODE}" ${_RESULT})

ENDMACRO (CHECK_STRUCT_MEMBER)
