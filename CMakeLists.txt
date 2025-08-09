cmake_minimum_required(VERSION 3.20)
project(pebbl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
  src/main.cpp
  src/parser/lexer/lexer.cpp
  src/parser/ast_generation/ast_generator.cpp
  src/runtime/gc.cpp
)

set(HEADER_DIRS
  src/parser/lexer
  src/parser/ast_generation
  src/runtime
  src/
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type")
endif()

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 OLD)
endif()
find_package(Boost REQUIRED)

add_executable(pebbli ${SOURCES})
target_include_directories(pebbli PRIVATE ${HEADER_DIRS} ${Boost_INCLUDE_DIRS})

target_link_libraries(pebbli ${Boost_LIBRARIES})

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

install(TARGETS pebbli DESTINATION bin)