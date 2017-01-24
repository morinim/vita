# Creates the "sr" executable.

# See <http://stackoverflow.com/q/6646405/3235496>
find_package (Boost REQUIRED COMPONENTS program_options)
include_directories (SYSTEM ${Boost_INCLUDE_DIRS})

file(GLOB_RECURSE SR_SRC "*.cc")

add_executable(sr ${SR_SRC})

target_link_libraries(sr vita ${Boost_LIBRARIES})
