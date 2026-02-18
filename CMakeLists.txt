cmake_minimum_required(VERSION 3.16)

project(multigauge_core LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ------------------------------------------------------------
# Core sources
# ------------------------------------------------------------
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(multigauge_core STATIC)
target_sources(multigauge_core PRIVATE ${CORE_SOURCES})

# ------------------------------------------------------------
# Third-party sources that must be compiled
# ------------------------------------------------------------

# lodepng
target_sources(multigauge_core PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/lib/lodepng/src/lodepng.cpp"
)

# tjpgd (C)
target_sources(multigauge_core PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/lib/tjpgd/src/tjpgd.c"
)

# yoga (C++)
file(GLOB_RECURSE YOGA_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/lib/yoga/src/yoga/*.cpp"
)
target_sources(multigauge_core PRIVATE ${YOGA_SOURCES})

# ------------------------------------------------------------
# Include directories
# ------------------------------------------------------------
# Public API headers (for consumers)
target_include_directories(multigauge_core PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Private/internal headers 
target_include_directories(multigauge_core PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src"

  # third-party headers
  "${CMAKE_CURRENT_SOURCE_DIR}/lib/rapidjson/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/lib/lodepng/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/lib/tjpgd/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/lib/yoga/src"
)