cmake_minimum_required(VERSION 3.16)

project(multigauge LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

option(MULTIGAUGE_BUILD_TESTS "Build multigauge core tests" ON)

include(FetchContent)

if(POLICY CMP0169)
  cmake_policy(SET CMP0169 OLD)
endif()

set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(
  rapidjson
  URL https://github.com/Tencent/rapidjson/archive/refs/tags/v1.1.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_Populate(rapidjson)
set(MULTIGAUGE_RAPIDJSON_INCLUDE_DIR "${rapidjson_SOURCE_DIR}/include")

FetchContent_Declare(
  doctest
  URL https://github.com/doctest/doctest/archive/refs/tags/v2.4.11.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_Populate(doctest)
set(MULTIGAUGE_DOCTEST_INCLUDE_DIR "${doctest_SOURCE_DIR}")

FetchContent_Declare(
  yoga
  URL https://github.com/facebook/yoga/archive/refs/tags/v2.0.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_Populate(yoga)
add_subdirectory("${yoga_SOURCE_DIR}/yoga" "${CMAKE_BINARY_DIR}/_deps/yoga")

FetchContent_Declare(
  lodepng
  URL https://github.com/lvandeve/lodepng/archive/ed6fe5825c6a4fbb7f58ab35a4231c7543cd452a.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_Populate(lodepng)

add_library(lodepng STATIC "${lodepng_SOURCE_DIR}/lodepng.cpp")
target_include_directories(lodepng PUBLIC "${lodepng_SOURCE_DIR}")

FetchContent_Declare(
  tjpgd
  URL https://github.com/cmumford/TJpgDec/archive/refs/tags/R0.03.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_Populate(tjpgd)

add_library(tjpgd STATIC "${tjpgd_SOURCE_DIR}/src/tjpgd.c")
target_include_directories(tjpgd PUBLIC "${tjpgd_SOURCE_DIR}/src")

file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(multigauge STATIC)
target_sources(multigauge PRIVATE ${CORE_SOURCES})

target_include_directories(multigauge PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${MULTIGAUGE_RAPIDJSON_INCLUDE_DIR}"
  "${lodepng_SOURCE_DIR}"
  "${tjpgd_SOURCE_DIR}/src"
)
target_include_directories(multigauge PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(multigauge PUBLIC yogacore)
target_link_libraries(multigauge PRIVATE lodepng)
target_link_libraries(multigauge PRIVATE tjpgd)

if(MULTIGAUGE_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
