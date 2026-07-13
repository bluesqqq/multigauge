cmake_minimum_required(VERSION 3.16)

project(multigauge LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ------------------------------------------------------------
# Third-party sources (Declarations)
# ------------------------------------------------------------
include(FetchContent)

# rapidjson
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
  URL_HASH SHA256=4842d03e91122be58fffa16bf248d9be70afbfb093156cfd7be1315b746c1ef5
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
FetchContent_MakeAvailable(rapidjson yoga)

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
  target_include_directories(tjpgd PUBLIC "${tjpgd_SOURCE_DIR}")
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

# ------------------------------------------------------------
# Link Dependencies
# ------------------------------------------------------------
target_link_libraries(multigauge PRIVATE yogacore)
target_link_libraries(multigauge PRIVATE rapidjson)
target_link_libraries(multigauge PRIVATE lodepng)
target_link_libraries(multigauge PRIVATE tjpgd)