cmake_minimum_required(VERSION 3.16)
project(multigauge_web LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT EMSCRIPTEN)
  message(FATAL_ERROR
    "multigauge_web requires the Emscripten toolchain.\n"
    "Configure with EMSDK set or use the wasm presets from ports/web."
  )
endif()

set(WEB_RUNTIME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/runtime")
set(WEB_DIST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dist")
set(MULTIGAUGE_CORE_DIR
  "${CMAKE_CURRENT_SOURCE_DIR}/../../core"
  CACHE PATH "Path to the multigauge-core checkout"
)

if (NOT EXISTS "${MULTIGAUGE_CORE_DIR}/CMakeLists.txt")
  message(FATAL_ERROR
    "multigauge-core not found at '${MULTIGAUGE_CORE_DIR}'.\n"
    "Expected the shared core to live at '../../core' from ports/web.\n"
    "Set MULTIGAUGE_CORE_DIR to a valid checkout if you want to use a different path."
  )
endif()

add_subdirectory("${MULTIGAUGE_CORE_DIR}" multigauge-core-build)

set(MG_WASM_SOURCES
  runtime/src/main.cpp
  runtime/bindings/ApiBindings.cpp
  runtime/bindings/EditorApi.cpp
  runtime/bindings/RuntimeApi.cpp
  runtime/src/platform/FileSystemEmscripten.cpp
  runtime/src/platform/GraphicsContextCanvas.cpp
  runtime/src/platform/LoggerWebConsole.cpp
  runtime/src/platform/TimeWeb.cpp
)

include("${CMAKE_CURRENT_SOURCE_DIR}/EditorApi.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/RuntimeAPI.cmake")

set(MG_WASM_EXPORTED_FUNCTIONS
  ${MG_RUNTIME_API_EXPORTS}
  ${MG_EDITOR_API_EXPORTS}
)
string(JOIN "," MG_WASM_EXPORTED_FUNCTIONS_JSON ${MG_WASM_EXPORTED_FUNCTIONS})

add_executable(multigauge_wasm ${MG_WASM_SOURCES})
target_link_libraries(multigauge_wasm PRIVATE multigauge)

set_target_properties(multigauge_wasm PROPERTIES
  OUTPUT_NAME "multigauge"
  RUNTIME_OUTPUT_DIRECTORY "${WEB_DIST_DIR}/wasm"
)

target_include_directories(multigauge_wasm PRIVATE
  ${WEB_RUNTIME_DIR}/bindings
  ${WEB_RUNTIME_DIR}/src
)

if (NOT MSVC)
  target_compile_options(multigauge_wasm PRIVATE
    -ftemplate-backtrace-limit=5
    -Wno-deprecated-declarations
  )
endif()

if (EMSCRIPTEN)
  set_target_properties(multigauge_wasm PROPERTIES SUFFIX ".js")

  target_link_options(multigauge_wasm PRIVATE
    "SHELL:-sWASM=1"
    "SHELL:-sENVIRONMENT=web"
    "SHELL:-sALLOW_MEMORY_GROWTH=1"
    "SHELL:-sASSERTIONS=1"
    "SHELL:-sFORCE_FILESYSTEM=1"
    "SHELL:-sEXPORTED_RUNTIME_METHODS=['FS','callMain','ccall','cwrap']"
    "SHELL:-sEXPORTED_FUNCTIONS=[${MG_WASM_EXPORTED_FUNCTIONS_JSON}]"
    "SHELL:-gsource-map"
  )

  add_custom_command(TARGET multigauge_wasm POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
      "${WEB_DIST_DIR}/js"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${WEB_RUNTIME_DIR}/js/pageRenderer.js"
      "${WEB_DIST_DIR}/js/pageRenderer.js"
    VERBATIM
  )
endif()
