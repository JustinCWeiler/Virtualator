cmake_minimum_required(VERSION 3.5)

project(virtualator C ASM)

set (SPEED "-Og")

set (SRCS
	src/main.c
	src/asm.S
	src/segfault/handler.c
)

set (CMAKE_C_STANDARD 99)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SPEED} -Wall -Wextra -Werror")

include_directories("${PROJECT_SOURCE_DIR}/include")

# zydis
option(ZYDIS_BUILD_TOOLS "" OFF)
option(ZYDIS_BUILD_EXAMPLES "" OFF)
option(ZYDIS_FEATURE_ENCODER "" OFF)
option(ZYDIS_FEATURE_FORMATTER "" OFF)
option(ZYDIS_FEATURE_AVX512 "" OFF)
option(ZYDIS_FEATURE_KNC "" OFF)
option(ZYDIS_FEATURE_SEGMENT "" OFF)
add_subdirectory("zydis")

# virtualator target
add_executable(virtualator ${SRCS})

target_link_libraries(virtualator PRIVATE "Zydis")
