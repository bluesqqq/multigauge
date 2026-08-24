cmake_minimum_required(VERSION 3.16)

project(multigauge LANGUAGES C CXX)

#----------[ LANGUAGE ]----------#

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Required by some older dependencies when using newer CMake versions.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

#----------[ OPTIONS ]----------#

option(
  MULTIGAUGE_BUILD_TESTS
  "Build Multigauge Core tests"
  ON
)

option(
  MULTIGAUGE_BUILD_EDITOR
  "Build editor APIs, screens, and editor-facing property metadata"
  ON
)

#----------[ DEPENDENCIES ]----------#

include(FetchContent)

if(POLICY CMP0169)
  cmake_policy(SET CMP0169 OLD)
endif()

set(FETCHCONTENT_QUIET OFF)

# ETL
FetchContent_Declare(
  etl
  GIT_REPOSITORY https://github.com/ETLCPP/etl.git
  GIT_TAG        20.48.1
  GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(etl)

# RapidJSON
FetchContent_Declare(
  rapidjson
  URL https://github.com/Tencent/rapidjson/archive/24b5e7a8b27f42fa16b96fc70aade9106cf7102.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_Populate(rapidjson)

set(
  MULTIGAUGE_RAPIDJSON_INCLUDE_DIR
  "${rapidjson_SOURCE_DIR}/include"
)

# doctest
FetchContent_Declare(
  doctest
  URL https://github.com/doctest/doctest/archive/refs/tags/v2.4.11.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_Populate(doctest)

set(
  MULTIGAUGE_DOCTEST_INCLUDE_DIR
  "${doctest_SOURCE_DIR}"
)

FetchContent_Declare(
  clay
  URL https://github.com/nicbarker/clay/archive/refs/tags/v0.14.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_Populate(clay)

# LodePNG
FetchContent_Declare(
  lodepng
  URL https://github.com/lvandeve/lodepng/archive/ed6fe5825c6a4fbb7f58ab35a4231c7543cd452a.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_Populate(lodepng)

add_library(
  lodepng
  STATIC
  "${lodepng_SOURCE_DIR}/lodepng.cpp"
)

target_include_directories(
  lodepng
  PUBLIC
    "${lodepng_SOURCE_DIR}"
)

# TJpgDec
FetchContent_Declare(
  tjpgd
  URL https://github.com/cmumford/TJpgDec/archive/refs/tags/R0.03.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_Populate(tjpgd)

add_library(
  tjpgd
  STATIC
  "${tjpgd_SOURCE_DIR}/src/tjpgd.c"
)

target_include_directories(
  tjpgd
  PUBLIC
    "${tjpgd_SOURCE_DIR}/src"
)

#----------[ MULTIGAUGE CORE ]----------#

file(
  GLOB_RECURSE
  CORE_SOURCES

  CONFIGURE_DEPENDS

  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

if(NOT MULTIGAUGE_BUILD_EDITOR)
  list(FILTER CORE_SOURCES EXCLUDE REGEX "/src/editor/.*\\.cpp$")
  list(FILTER CORE_SOURCES EXCLUDE REGEX "/src/screens/EditorScreen\\.cpp$")
endif()

add_library(multigauge STATIC)

target_sources(
  multigauge
  PRIVATE
    ${CORE_SOURCES}
)

target_include_directories(
  multigauge

  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${MULTIGAUGE_RAPIDJSON_INCLUDE_DIR}"
    "${lodepng_SOURCE_DIR}"
    "${tjpgd_SOURCE_DIR}/src"

  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${clay_SOURCE_DIR}"
)

target_link_libraries(
  multigauge

  PRIVATE
    etl::etl
    lodepng
    tjpgd
)

target_compile_definitions(
  multigauge
  PUBLIC
    MG_BUILD_EDITOR=$<BOOL:${MULTIGAUGE_BUILD_EDITOR}>
)

#----------[ TESTS ]----------#

if(MULTIGAUGE_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
