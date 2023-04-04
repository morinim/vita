# Creates the "imgui" library.

find_package(SDL2 REQUIRED)

if (${SDL2_FOUND})
  include_directories(${SDL2_INCLUDE_DIRS})
  include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR})

  file(GLOB_RECURSE IMGUI_SRC CONFIGURE_DEPENDS "*.cpp")

  if (CMAKE_CXX_COMPILER_ID MATCHES "GNU"
      OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(${NO_WARN_FLAGS})
  endif ()

  add_library(imgui ${IMGUI_SRC})
  target_link_libraries(imgui ${SDL2_LIBRARIES})
endif ()
