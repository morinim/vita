# Creates the examples.

file(GLOB FOREX_SRC "forex/*.cc")
add_executable(forex ${FOREX_SRC})
target_link_libraries(forex vita)

file(GLOB EXAMPLES_SRC *.cc)
foreach (example_src ${EXAMPLES_SRC})
  get_filename_component(example ${example_src} NAME_WE)
  add_executable(${example} ${example_src})
  target_link_libraries(${example} vita)
endforeach()
