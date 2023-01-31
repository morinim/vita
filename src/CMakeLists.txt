cmake_minimum_required(VERSION 3.20)

project(VITA)

#include(CheckIPOSupported)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

#if (CMAKE_BUILD_TYPE MATCHES "Release")
#check_ipo_supported(RESULT supported_ipo OUTPUT error_ipo)
#
#if (supported_ipo)
#  message(STATUS "IPO / LTO supported: <${supported_ipo}>")
#  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
#else ()
#  message(STATUS "IPO / LTO not supported: <${error_ipo}>")
#endif()
#endif()

#message("CMAKE_CXX_FLAGS_DEBUG is ${CMAKE_CXX_FLAGS_DEBUG}")
#message("CMAKE_CXX_FLAGS_RELEASE is ${CMAKE_CXX_FLAGS_RELEASE}")

# The general idea is to use the default values and overwrite them only for
# specific, well experimented environments.
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU"
    OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")

  set(NO_WARN_FLAGS "-w")

elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")

  set(NO_WARN_FLAGS "/w")

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
