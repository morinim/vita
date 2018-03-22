# Creates the "sr" executable.

file(GLOB_RECURSE SR_SRC "*.cc")

add_executable(sr ${SR_SRC})

target_link_libraries(sr vita docopt)
