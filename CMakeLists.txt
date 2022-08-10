cmake_minimum_required(VERSION 3.5)

project(virtualator)

set (SPEED "-Og")
set (XEDDIR "${PROJECT_SOURCE_DIR}/buildxed")

set (SRCS
	src/main.c
	src/segfault/handler.c
)

set (CMAKE_C_STANDARD 99)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SPEED} -Wall -Wextra -Werror")

include_directories("${PROJECT_SOURCE_DIR}/include")
include_directories("${XEDDIR}/include")

# xed target
add_custom_command(
	OUTPUT ${XEDDIR}
	DEPENDS ${PROJECT_SOURCE_DIR}/xed/mfile.py
	COMMAND ${PROJECT_SOURCE_DIR}/xed/mfile.py --no-encoder --opt=3 --limit-strings --install-dir ${XEDDIR} install
)

add_custom_target(
	libxed
	DEPENDS ${XEDDIR}
)

# virtualator target
add_executable(virtualator ${SRCS})

add_dependencies(virtualator libxed)
target_link_libraries(virtualator ${XEDDIR}/lib/libxed.a)
