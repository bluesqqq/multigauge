cmake_minimum_required(VERSION 3.16)

project(multigauge LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# ------------------------------------------------------------
# Third-party sources (Declarations)
# ------------------------------------------------------------
include(FetchContent)

# rapidjson
set(RAPIDJSON_BUILD_DOC OFF CACHE BOOL "Build rapidjson documentation." FORCE)
set(RAPIDJSON_BUILD_EXAMPLES OFF CACHE BOOL "Build rapidjson examples." FORCE)
set(RAPIDJSON_BUILD_TESTS OFF CACHE BOOL "Build rapidjson perftests and unittests." FORCE)
set(RAPIDJSON_BUILD_THIRDPARTY_GTEST OFF CACHE BOOL "Use gtest installation in `thirdparty/gtest` by default if available" FORCE)
FetchContent_Declare(
  rapidjson
  GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
  GIT_TAG        v1.1.0
)

# yoga
FetchContent_Declare(
  yoga
  GIT_REPOSITORY https://github.com/facebook/yoga.git
  GIT_TAG        v2.0.0
)

# lodepng 
FetchContent_Declare(
  lodepng
  URL      https://raw.githubusercontent.com/lvandeve/lodepng/master/lodepng.cpp
  URL_HASH SHA256=d98e1f40d303c1038a096ebf93b413a565a91cf2c72b9d2fa5c625c4279c3cb6
  DOWNLOAD_NO_EXTRACT TRUE
)

# tjpgd 
FetchContent_Declare(
  tjpgd
  URL      https://raw.githubusercontent.com/cmumford/TJpgDec/master/src/tjpgd.c
  DOWNLOAD_NO_EXTRACT TRUE
)

# ------------------------------------------------------------
# Process & Populate Dependencies
# ------------------------------------------------------------
FetchContent_MakeAvailable(rapidjson)

FetchContent_GetProperties(yoga)
if(NOT yoga_POPULATED)
  FetchContent_Populate(yoga)
  add_subdirectory("${yoga_SOURCE_DIR}/yoga" "${yoga_BINARY_DIR}/yoga")
endif()

FetchContent_GetProperties(lodepng)
if(NOT lodepng_POPULATED)
  FetchContent_Populate(lodepng)
  file(DOWNLOAD 
    https://raw.githubusercontent.com/lvandeve/lodepng/master/lodepng.h
    "${lodepng_SOURCE_DIR}/lodepng.h"
  )
  add_library(lodepng STATIC "${lodepng_SOURCE_DIR}/lodepng.cpp")
  target_include_directories(lodepng PUBLIC "${lodepng_SOURCE_DIR}")
endif()

FetchContent_GetProperties(tjpgd)
if(NOT tjpgd_POPULATED)
  FetchContent_Populate(tjpgd)
  file(DOWNLOAD 
    https://raw.githubusercontent.com/cmumford/TJpgDec/master/src/tjpgd.h
    "${tjpgd_SOURCE_DIR}/tjpgd.h"
  )
  add_library(tjpgd STATIC "${tjpgd_SOURCE_DIR}/tjpgd.c")
  target_compile_options(tjpgd PRIVATE
    $<$<C_COMPILER_ID:MSVC>:/U_WIN32>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-U_WIN32>
  )
  target_include_directories(tjpgd PUBLIC
    "${tjpgd_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/tjpgd"
  )
endif()

# ------------------------------------------------------------
# Core Target
# ------------------------------------------------------------
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(multigauge STATIC)
target_sources(multigauge PRIVATE ${CORE_SOURCES})

target_include_directories(multigauge PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_include_directories(multigauge PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
FetchContent_GetProperties(rapidjson)
if(rapidjson_POPULATED)
  target_include_directories(multigauge PUBLIC
    "${rapidjson_SOURCE_DIR}/include"
  )
endif()

# ------------------------------------------------------------
# Link Dependencies
# ------------------------------------------------------------
target_link_libraries(multigauge PRIVATE yogacore)
target_link_libraries(multigauge PRIVATE rapidjson)
target_link_libraries(multigauge PRIVATE lodepng)
target_link_libraries(multigauge PRIVATE tjpgd)
