# Creates the tests.

# See <http://stackoverflow.com/q/6646405/3235496> and
# <https://stackoverflow.com/q/28887680> for details about linking Boost.

find_package (Boost REQUIRED COMPONENTS unit_test_framework)
include_directories (SYSTEM ${Boost_INCLUDE_DIRS})

file(GLOB TESTS_SRC *.cc)
foreach (test_src ${TESTS_SRC})
  get_filename_component(test ${test_src} NAME_WE)
  add_executable(${test} ${test_src})
  if (WIN32)
    target_compile_definitions(${test} PRIVATE BOOST_ALL_NO_LIB)
  endif()
  target_link_libraries(${test} vita ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
endforeach()

# Resources needed for testing
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_resources
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
