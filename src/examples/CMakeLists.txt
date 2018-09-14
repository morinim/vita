# Creates the examples.

add_subdirectory(sr)

file(GLOB FOREX_SRC "forex/*.cc")
add_executable(forex ${FOREX_SRC})
target_link_libraries(forex vita)

file(GLOB EXAMPLES_SRC *.cc)
foreach (example_src ${EXAMPLES_SRC})
  # Gets the filename (`example`) without directory / longest extension
  # (`NAME_WE`) from the full filename (${example_src})
  get_filename_component(example ${example_src} NAME_WE)

  add_executable(${example} ${example_src})
  target_link_libraries(${example} vita)
endforeach()

# Resources needed by examples
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/titanic_train.csv
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
