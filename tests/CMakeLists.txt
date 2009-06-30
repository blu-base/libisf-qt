include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

macro(add_multiple_tests)
  foreach(test ${ARGN})
    qt4_automoc(test_${test}.cpp)
    add_executable(test_${test} test_${test}.cpp)
    add_test(libisf-${test} test_${test})
    target_link_libraries(test_${test} ${QT_QTTEST_LIBRARY} ${QT_QTCORE_LIBRARY} isf-qt )
  endforeach(test)
endmacro(add_multiple_tests)

add_multiple_tests(
  isfdrawing
  multibyte_coding
)

#set(libisfTest_SRCS libisfTest.cpp)
#qt4_automoc(${libisfTest_SRCS})
#add_executable(libisfTest ${libisfTest_SRCS})
#add_test(libisfTest libisfTest)
#target_link_libraries(libisfTest ${QT_QTTEST_LIBRARY})
