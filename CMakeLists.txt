cmake_minimum_required(VERSION 3.24)

project(
    ccompiler
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
    )
endif()

# ------------------------------------------------------------------------------
# Third-party libraries
# ------------------------------------------------------------------------------

add_library(matchit INTERFACE)

target_include_directories(matchit INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/external/BowenFu
)

# ------------------------------------------------------------------------------
# Main executable
# ------------------------------------------------------------------------------

add_executable(ccompiler)

target_sources(ccompiler PRIVATE
    src/main.cpp
)

target_include_directories(ccompiler PRIVATE
    include
)

target_link_libraries(ccompiler PRIVATE
    matchit
)
