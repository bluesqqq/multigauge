cmake_minimum_required(VERSION 3.16)

project(multigauge LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/Dependencies.cmake")
multigauge_bootstrap_core_dependencies()

set(MULTIGAUGE_RAPIDJSON_INCLUDE_DIR "${MULTIGAUGE_DEPS_DIR}/rapidjson/include")
set(MULTIGAUGE_YOGA_SOURCE_DIR "${MULTIGAUGE_DEPS_DIR}/yoga/yoga")
set(MULTIGAUGE_LODEPNG_SOURCE_DIR "${MULTIGAUGE_DEPS_DIR}/lodepng")
set(MULTIGAUGE_TJPGD_SOURCE_DIR "${MULTIGAUGE_DEPS_DIR}/tjpgd")

add_subdirectory("${MULTIGAUGE_YOGA_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/_deps/yoga")

add_library(lodepng STATIC "${MULTIGAUGE_LODEPNG_SOURCE_DIR}/lodepng.cpp")
target_include_directories(lodepng PUBLIC "${MULTIGAUGE_LODEPNG_SOURCE_DIR}")

add_library(tjpgd STATIC "${MULTIGAUGE_TJPGD_SOURCE_DIR}/tjpgd.c")
target_compile_options(tjpgd PRIVATE
  $<$<C_COMPILER_ID:MSVC>:/U_WIN32>
  $<$<NOT:$<C_COMPILER_ID:MSVC>>:-U_WIN32>
)
target_include_directories(tjpgd PUBLIC "${MULTIGAUGE_TJPGD_SOURCE_DIR}")

file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(multigauge STATIC)
target_sources(multigauge PRIVATE ${CORE_SOURCES})

target_include_directories(multigauge PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${MULTIGAUGE_RAPIDJSON_INCLUDE_DIR}"
)
target_include_directories(multigauge PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(multigauge PUBLIC yogacore)
target_link_libraries(multigauge PRIVATE lodepng)
target_link_libraries(multigauge PRIVATE tjpgd)
