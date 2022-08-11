cmake_minimum_required(VERSION 3.5)

project(virtualator)

set (SPEED "-Og")

set (SRCS
	src/main.c
	src/segfault/handler.c
)

set (CMAKE_C_STANDARD 99)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SPEED} -Wall -Wextra -Werror")

include_directories("${PROJECT_SOURCE_DIR}/include")

# zydis
option(ZYDIS_BUILD_TOOLS "" OFF)
option(ZYDIS_BUILD_EXAMPLES "" OFF)
add_subdirectory("zydis")

# virtualator target
add_executable(virtualator ${SRCS})

target_link_libraries(virtualator PRIVATE "Zydis")
