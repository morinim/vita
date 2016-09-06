# Creates the tests.

# See <http://stackoverflow.com/q/6646405/3235496>
find_package (Boost REQUIRED COMPONENTS unit_test_framework)
include_directories (SYSTEM ${Boost_INCLUDE_DIRS})

file(GLOB TESTS_SRC *.cc)
foreach (test_src ${TESTS_SRC})
  get_filename_component(test ${test_src} NAME_WE)
  add_executable(${test} ${test_src})
  target_compile_definitions(${test} PRIVATE "BOOST_TEST_DYN_LINK=1")
  target_link_libraries(${test} vita ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
endforeach()
