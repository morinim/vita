# Creates the "tinyxml2" library.

set(FRAMEWORK_SRC tinyxml2.cc)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
add_compile_options(${NO_WARN_FLAGS})
endif ()

add_library(tinyxml2 ${FRAMEWORK_SRC})
