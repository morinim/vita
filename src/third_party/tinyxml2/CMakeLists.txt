# Creates the "tinyxml2" library.

set(TINYXML2_SRC tinyxml2.cc)

add_compile_options(${NO_WARN_FLAGS})

add_library(tinyxml2 ${TINYXML2_SRC})
