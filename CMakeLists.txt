cmake_minimum_required(VERSION 3.20)
project(pebbl C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(SOURCES 
  interpreter/bytecode/chunk.c
  interpreter/runtime/value.c
  interpreter/runtime/vm.c
  interpreter/main.c
  utils/debug.c
)

set(HEADER_DIRS
  interpreter/bytecode
  interpreter/runtime
  utils
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

add_executable(pebbl ${SOURCES})
target_include_directories(pebbl PRIVATE ${HEADER_DIRS})

if (MSVC)
  target_compile_options(pebbl PRIVATE
    /W4
    /WX
    $<$<CONFIG:Release>:/O2>
    $<$<CONFIG:Debug>:/Od>
    $<$<CONFIG:Debug>:/Zi>
  )
else()
  target_compile_options(pebbl PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Debug>:-g>
  )
endif()
