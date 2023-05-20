# Creates the "gp_pathfinder" executable.

find_package(SDL2 REQUIRED)

if (${SDL2_FOUND})
  include_directories(${SDL2_INCLUDE_DIRS})

  file(GLOB_RECURSE GP_PATHFINDER_SRC CONFIGURE_DEPENDS "*.cc")

  find_package (Threads)

  add_executable(gp_pathfinder ${GP_PATHFINDER_SRC})
  target_link_libraries(gp_pathfinder vita imgui ${SDL2_LIBRARIES} Threads::Threads)

  # Resources needed by examples
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/map.txt
       DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()
