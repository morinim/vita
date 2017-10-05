cmake_minimum_required(VERSION 3.5)

project(VITA)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_COMPILER_IS_CLANGXX 1)
endif ()

# The general idea is to use the default values and overwrite them only for
# specific, well experimented systems.
if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
set(NO_WARN_FLAGS "-w")
set(WARN_FLAGS "-Wpedantic" "-Wall" "-Wextra" "-Winvalid-pch" "-Wpedantic"
               "-Wformat=2" "-Wfloat-equal" "-Wshadow")
set(OTHER_FLAGS "-pipe" "-march=native")

set(CMAKE_CXX_FLAGS_DEBUG "-Og")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -DBOOST_DISABLE_ASSERTS")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-s")
endif ()

add_compile_options(${OTHER_FLAGS} ${WARN_FLAGS})

# Compiler must support the C++14 standard.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${VITA_SOURCE_DIR})
include_directories(SYSTEM ${VITA_SOURCE_DIR}/third_party/)

# Recurse into the following subdirectories (this does not actually cause
# another cmake executable to run. The same process will walk through the
# project's entire directory structure).
add_subdirectory(third_party/tinyxml2)
add_subdirectory(kernel)
add_subdirectory(examples)
add_subdirectory(test)
