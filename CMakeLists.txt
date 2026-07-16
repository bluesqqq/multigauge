cmake_minimum_required(VERSION 3.16)

project(multigauge_superbuild LANGUAGES NONE)

include(ExternalProject)

option(MULTIGAUGE_BUILD_CORE "Build the native core library" ON)
option(MULTIGAUGE_BUILD_WEB "Build the web port" ON)
option(MULTIGAUGE_REQUIRE_WEB "Fail if the web port cannot be configured" OFF)

set(MULTIGAUGE_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}")
function(multigauge_append_common_args out_var fetchcontent_base_dir)
  set(args
    "-DFETCHCONTENT_BASE_DIR:PATH=${fetchcontent_base_dir}"
  )

  if(CMAKE_BUILD_TYPE)
    list(APPEND args "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}")
  endif()

  set(${out_var} "${args}" PARENT_SCOPE)
endfunction()

function(multigauge_resolve_emscripten_toolchain out_var)
  if(DEFINED ENV{EMSDK} AND NOT "$ENV{EMSDK}" STREQUAL "")
    set(candidate "$ENV{EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake")
    if(EXISTS "${candidate}")
      set(${out_var} "${candidate}" PARENT_SCOPE)
      return()
    endif()
  endif()

  set(${out_var} "" PARENT_SCOPE)
endfunction()

if(MULTIGAUGE_BUILD_CORE)
  set(MULTIGAUGE_CORE_FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/core/_deps")
  multigauge_append_common_args(MULTIGAUGE_CORE_CMAKE_ARGS
    "${MULTIGAUGE_CORE_FETCHCONTENT_BASE_DIR}"
  )

  ExternalProject_Add(multigauge_external
    SOURCE_DIR "${MULTIGAUGE_SOURCE_DIR}/core"
    BINARY_DIR "${CMAKE_BINARY_DIR}/core"
    CMAKE_ARGS
      ${MULTIGAUGE_CORE_CMAKE_ARGS}
    INSTALL_COMMAND ""
  )
endif()

set(MULTIGAUGE_WEB_TOOLCHAIN_FILE "")
if(MULTIGAUGE_BUILD_WEB)
  multigauge_resolve_emscripten_toolchain(MULTIGAUGE_WEB_TOOLCHAIN_FILE)
  if(MULTIGAUGE_WEB_TOOLCHAIN_FILE)
    set(MULTIGAUGE_WEB_FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/ports/web/_deps")
    multigauge_append_common_args(MULTIGAUGE_WEB_CMAKE_ARGS
      "${MULTIGAUGE_WEB_FETCHCONTENT_BASE_DIR}"
    )

    ExternalProject_Add(multigauge_web_external
      SOURCE_DIR "${MULTIGAUGE_SOURCE_DIR}/ports/web"
      BINARY_DIR "${CMAKE_BINARY_DIR}/ports/web"
      CMAKE_GENERATOR Ninja
      CMAKE_ARGS
        ${MULTIGAUGE_WEB_CMAKE_ARGS}
        "-DCMAKE_TOOLCHAIN_FILE:FILEPATH=${MULTIGAUGE_WEB_TOOLCHAIN_FILE}"
        "-DMULTIGAUGE_CORE_DIR:PATH=${MULTIGAUGE_SOURCE_DIR}/core"
      INSTALL_COMMAND ""
      DEPENDS multigauge_external
    )
  elseif(MULTIGAUGE_REQUIRE_WEB)
    message(FATAL_ERROR
      "MULTIGAUGE_BUILD_WEB is ON, but emsdk was not found.\n"
      "Set EMSDK or place emsdk next to the repo root so the web port can be configured."
    )
  else()
    message(WARNING
      "Skipping web build because emsdk was not found. Set EMSDK to enable the web port."
    )
  endif()
endif()

add_custom_target(multigauge_ci ALL)

if(TARGET multigauge_external)
  add_dependencies(multigauge_ci multigauge_external)
endif()

if(TARGET multigauge_web_external)
  add_dependencies(multigauge_ci multigauge_web_external)
endif()
