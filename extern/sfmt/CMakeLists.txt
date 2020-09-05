cmake_minimum_required(VERSION 3.17)
project(sfmt_project)

include("${CMAKE_SOURCE_DIR}/cmake/FindSSE.cmake")
FindSSE()

add_library(sfmt
  "${CMAKE_CURRENT_SOURCE_DIR}/SFMT.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/SFMT.h")

target_include_directories(sfmt
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}     
)


if(SSE2_FOUND)
  if(MSVC)
    target_compile_options(sfmt PRIVATE /arch:SSE2)
  else()
    target_compile_options(sfmt PRIVATE -msse2)
  endif()
endif()


