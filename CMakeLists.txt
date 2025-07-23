cmake_minimum_required(VERSION 3.20)
project(pebbl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
  src/utils/debug.cpp
  src/parser/lexer/lexer.cpp
)

set(HEADER_DIRS
  src/parser/lexer
  src/parser/ast_generation
  src/utils
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

add_executable(pebbli ${SOURCES})
target_include_directories(pebbli PRIVATE ${HEADER_DIRS})

if (MSVC)
  target_compile_options(pebbli PRIVATE  
    /W4
    /WX
    $<$<CONFIG:Release>:/O2>
    $<$<CONFIG:Debug>:/Od>
    $<$<CONFIG:Debug>:/Zi>
  )
else()
  target_compile_options(pebbli PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Debug>:-g>
  )
endif()
