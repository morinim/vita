# Creates the tests.

enable_testing()

file(GLOB TESTS_SRC *.cc)
foreach (test_src ${TESTS_SRC})
  get_filename_component(test ${test_src} NAME_WE)
  add_executable(${test} ${test_src})
  target_link_libraries(${test} vita)

  if ((NOT ${test} STREQUAL "tests")
      AND (NOT ${test} MATCHES "^speed_"))
    add_test(NAME ${test} COMMAND ${test})
  endif()
endforeach()

# Resources needed for testing
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_resources
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
