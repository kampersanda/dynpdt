cmake_minimum_required(VERSION 2.6)
project(DYNPDT)

set(CMAKE_CXX_FLAGS "-Wall -std=c++14")

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif ()

configure_file(
  ${DYNPDT_SOURCE_DIR}/dynpdt_config.hpp.in
  ${DYNPDT_SOURCE_DIR}/include/dynpdt_config.hpp
)

option(DYNPDT_USE_POPCNT
  "Use popcount intrinsic. Available on x86-64 since SSE4.2."
  OFF)

if (DYNPDT_USE_POPCNT)
  if (UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2")
  endif ()
endif ()

message(STATUS "BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
message(STATUS "CXX_FLAGS are ${CMAKE_CXX_FLAGS}")
message(STATUS "CXX_FLAGS_DEBUG are ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CXX_FLAGS_RELEASE are ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "DYNPDT_USE_POPCNT is ${DYNPDT_USE_POPCNT}")

include_directories(include)

# For links in my personal IDE
set(HEADERS
  include/basics.hpp
  include/bit_tools.hpp
  include/DynPDT.hpp
  include/FitVector.hpp
  include/LabelPool_BitMap.hpp
  include/LabelPool_Plain.hpp
  include/SimpleBonsai.hpp
  include/vbyte.hpp
  )

add_executable(bench bench.cpp ${HEADERS})

enable_testing()
file(GLOB TEST_SOURCES test_*.cpp)

foreach (TEST_SOURCE ${TEST_SOURCES})
  get_filename_component(TEST_SOURCE_NAME ${TEST_SOURCE} NAME_WE)
  add_executable(${TEST_SOURCE_NAME} ${TEST_SOURCE})
  add_test(${TEST_SOURCE_NAME} ${TEST_SOURCE_NAME})
endforeach ()
