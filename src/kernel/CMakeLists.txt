# Creates the "vita" library.

file(GLOB_RECURSE KERNEL_SRC "*.cc")
file(GLOB_RECURSE UTILITY_SRC "../utility/*.cc")

set(FRAMEWORK_SRC ${KERNEL_SRC} ${UTILITY_SRC})

add_library(vita ${FRAMEWORK_SRC})

target_link_libraries(vita tinyxml2)

add_custom_command(TARGET vita POST_BUILD
                   COMMAND ../tools/single_include.py --src-include-dir ./ --src-include kernel/vita.h --dst-include ${CMAKE_CURRENT_BINARY_DIR}/auto_vita.h
				   WORKING_DIRECTORY ${VITA_SOURCE_DIR}
				   VERBATIM)

include(GNUInstallDirs)
install(TARGETS vita)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/auto_vita.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
		RENAME vita.h)
