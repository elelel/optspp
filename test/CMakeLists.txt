cmake_minimum_required(VERSION 3.4.0)
project(optspp_test)

get_filename_component(TOP_DIR "${CMAKE_CURRENT_SOURCE_DIR}" PATH)
set(CATCH "${TOP_DIR}/contrib/Catch")

include_directories(
  ${TOP_DIR}/include
  ${CATCH}/include
)

set(TEST_SOURCES
  optspp.cpp)

add_executable(optspp_test
  ${TEST_SOURCES})

target_compile_definitions(optspp_test PUBLIC "CATCH_CONFIG_MAIN" "CATCH_CONFIG_COLOUR_NONE")
add_test(CommonTests optspp_test)
