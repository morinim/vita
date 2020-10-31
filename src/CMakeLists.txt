cmake_minimum_required(VERSION 3.12)

project(VITA)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# The general idea is to use the default values and overwrite them only for
# specific, well experimented environments.
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU"
    OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")

  set(NO_WARN_FLAGS "-w")

  set(WARN_FLAGS "-Wpedantic" "-Wall" "-Wextra" "-Winvalid-pch" "-Wpedantic"
                 "-Wformat=2" "-Wfloat-equal" "-Wshadow" "-Wdouble-promotion"
                 "-Wzero-as-null-pointer-constant")
  # `-Wconversion`, `-Wsign-conversion`, `-Weffc++` are other interesting
  # switches to try from time to time (they give many false positives).

  set(OTHER_FLAGS "-pipe" "-march=native")

  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -DBOOST_DISABLE_ASSERTS")

  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-s")

elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")

  set(NO_WARN_FLAGS "/w")
  set(WARN_FLAGS "/W3")

endif()

add_compile_options(${OTHER_FLAGS} ${WARN_FLAGS})

# Compiler must support the C++17 standard.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${VITA_SOURCE_DIR})
include_directories(SYSTEM ${VITA_SOURCE_DIR}/third_party/)

# Recurse into the following subdirectories (this does not actually cause
# another cmake executable to run. The same process will walk through the
# project's entire directory structure).
add_subdirectory(third_party/docopt)
add_subdirectory(third_party/tinyxml2)
add_subdirectory(kernel)
add_subdirectory(examples)
add_subdirectory(test)
