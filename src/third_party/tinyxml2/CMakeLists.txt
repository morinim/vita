# Creates the "tinyxml2" library.

set(TINYXML2_SRC tinyxml2.cc)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU"
    OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
add_compile_options(${NO_WARN_FLAGS})
endif ()

add_library(tinyxml2 ${TINYXML2_SRC})
