# Creates the "vita" library.

file(GLOB_RECURSE KERNEL_SRC "*.cc")
file(GLOB_RECURSE UTILITY_SRC "../utility/*.cc")

set(FRAMEWORK_SRC ${KERNEL_SRC} ${UTILITY_SRC})

add_library(vita ${FRAMEWORK_SRC})

target_link_libraries(vita tinyxml2)

install(TARGETS vita DESTINATION lib)
install(DIRECTORY ./ DESTINATION include/vita/kernel FILES_MATCHING PATTERN "*.h" PATTERN "*.tcc")
