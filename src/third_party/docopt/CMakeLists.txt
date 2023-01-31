# Creates the "docopt" library.

set(DOCOPT_SRC docopt.cc)

add_compile_options(${NO_WARN_FLAGS})

add_library(docopt ${DOCOPT_SRC})
