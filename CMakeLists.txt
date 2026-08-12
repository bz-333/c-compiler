cmake_minimum_required(VERSION 3.24)

project(
    ccompiler
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compilation database for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

target_include_directories(matchit SYSTEM INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/external/BowenFu
)

# ------------------------------------------------------------------------------
# Compiler library
# ------------------------------------------------------------------------------

add_library(compiler STATIC
    src/compiler/lexer.cpp
    src/compiler/parser.cpp
    src/compiler/ast_pprint.cpp
    src/compiler/tacky_pprint.cpp
    src/compiler/irgen.cpp
    src/compiler/codegen.cpp
    src/compiler/asmgen.cpp
    src/compiler/emit.cpp
)

target_include_directories(compiler PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(compiler PUBLIC
    matchit
)

# ------------------------------------------------------------------------------
# Driver executable
# ------------------------------------------------------------------------------

add_executable(ccompiler)

target_sources(ccompiler PRIVATE
    src/main.cpp
    src/driver.cpp
)

target_link_libraries(ccompiler PRIVATE
    compiler
)
