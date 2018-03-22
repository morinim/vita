# Creates the "docopt" library.

set(DOCOPT_SRC docopt.cc)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
add_compile_options(${NO_WARN_FLAGS})
endif ()

add_library(docopt ${DOCOPT_SRC})
