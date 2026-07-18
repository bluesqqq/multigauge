if(DEFINED MULTIGAUGE_DEPENDENCIES_INCLUDED)
  return()
endif()
set(MULTIGAUGE_DEPENDENCIES_INCLUDED TRUE)

if(NOT DEFINED MULTIGAUGE_SOURCE_DIR)
  get_filename_component(MULTIGAUGE_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
endif()

if(NOT DEFINED MULTIGAUGE_DEPS_DIR)
  set(MULTIGAUGE_DEPS_DIR "${MULTIGAUGE_SOURCE_DIR}/.deps")
endif()

set(MULTIGAUGE_CORE_LIB_DIR "${MULTIGAUGE_SOURCE_DIR}/core/lib")
set(MULTIGAUGE_ESP32_LIB_DIR "${MULTIGAUGE_SOURCE_DIR}/ports/esp32/lib")

function(multigauge_download_file url destination)
  if(EXISTS "${destination}")
    return()
  endif()

  get_filename_component(destination_dir "${destination}" DIRECTORY)
  file(MAKE_DIRECTORY "${destination_dir}")

  message(STATUS "Downloading ${url}")
  file(DOWNLOAD
    "${url}"
    "${destination}"
    SHOW_PROGRESS
    STATUS download_status
    TLS_VERIFY ON
  )

  list(GET download_status 0 download_code)
  list(GET download_status 1 download_message)
  if(NOT download_code EQUAL 0)
    message(FATAL_ERROR "Failed to download ${url}: ${download_message}")
  endif()
endfunction()

function(multigauge_clone_repo url tag destination)
  if(EXISTS "${destination}/.git")
    return()
  endif()

  if(EXISTS "${destination}/.multigauge-ready")
    return()
  endif()

  if(EXISTS "${destination}")
    return()
  endif()

  get_filename_component(destination_parent "${destination}" DIRECTORY)
  file(MAKE_DIRECTORY "${destination_parent}")

  message(STATUS "Cloning ${url} (${tag})")
  execute_process(
    COMMAND git clone --depth 1 --branch "${tag}" "${url}" "${destination}"
    RESULT_VARIABLE clone_result
  )

  if(NOT clone_result EQUAL 0)
    message(FATAL_ERROR "Failed to clone ${url} (${tag})")
  endif()

  file(WRITE "${destination}/.multigauge-ready" "ok\n")
endfunction()

function(multigauge_copy_or_link_directory target link)
  if(EXISTS "${link}")
    return()
  endif()

  get_filename_component(link_parent "${link}" DIRECTORY)
  file(MAKE_DIRECTORY "${link_parent}")

  if(WIN32)
    execute_process(
      COMMAND powershell -NoProfile -ExecutionPolicy Bypass -Command "New-Item -ItemType Junction -Path '${link}' -Target '${target}' | Out-Null"
      RESULT_VARIABLE link_result
      OUTPUT_QUIET
      ERROR_QUIET
    )
  else()
    execute_process(
      COMMAND "${CMAKE_COMMAND}" -E create_symlink "${target}" "${link}"
      RESULT_VARIABLE link_result
      OUTPUT_QUIET
      ERROR_QUIET
    )
  endif()

  if(NOT link_result EQUAL 0)
    message(STATUS "Symlink creation failed for ${link}; copying directory instead.")
    file(COPY "${target}" DESTINATION "${link_parent}")
    get_filename_component(target_name "${target}" NAME)
    if(NOT "${target_name}" STREQUAL "${link}")
      if(EXISTS "${link_parent}/${target_name}" AND NOT EXISTS "${link}")
        file(RENAME "${link_parent}/${target_name}" "${link}")
      endif()
    endif()
  endif()
endfunction()

function(multigauge_copy_file target link)
  if(EXISTS "${link}")
    return()
  endif()

  get_filename_component(link_parent "${link}" DIRECTORY)
  file(MAKE_DIRECTORY "${link_parent}")
  file(COPY "${target}" DESTINATION "${link_parent}")
endfunction()

function(multigauge_patch_rapidjson rapidjson_include_dir)
  set(document_header "${rapidjson_include_dir}/rapidjson/document.h")
  if(NOT EXISTS "${document_header}")
    return()
  endif()

  file(READ "${document_header}" document_content)
  set(broken_assignment
    "GenericStringRef& operator=(const GenericStringRef& rhs) { s = rhs.s; length = rhs.length; }"
  )
  string(FIND "${document_content}" "${broken_assignment}" broken_assignment_index)
  if(NOT broken_assignment_index EQUAL -1)
    string(REPLACE
      "${broken_assignment}"
      "GenericStringRef& operator=(const GenericStringRef& rhs) = delete;"
      patched_content
      "${document_content}"
    )
    file(WRITE "${document_header}" "${patched_content}")
  endif()
endfunction()

function(multigauge_bootstrap_dependencies)
  file(MAKE_DIRECTORY "${MULTIGAUGE_DEPS_DIR}")

  # Shared core dependencies.
  multigauge_clone_repo(
    "https://github.com/Tencent/rapidjson.git"
    "v1.1.0"
    "${MULTIGAUGE_DEPS_DIR}/rapidjson"
  )
  multigauge_patch_rapidjson("${MULTIGAUGE_DEPS_DIR}/rapidjson/include")

  multigauge_clone_repo(
    "https://github.com/facebook/yoga.git"
    "v2.0.0"
    "${MULTIGAUGE_DEPS_DIR}/yoga"
  )

  multigauge_download_file(
    "https://raw.githubusercontent.com/lvandeve/lodepng/master/lodepng.cpp"
    "${MULTIGAUGE_DEPS_DIR}/lodepng/lodepng.cpp"
  )
  multigauge_download_file(
    "https://raw.githubusercontent.com/lvandeve/lodepng/master/lodepng.h"
    "${MULTIGAUGE_DEPS_DIR}/lodepng/lodepng.h"
  )

  multigauge_download_file(
    "https://raw.githubusercontent.com/cmumford/TJpgDec/master/src/tjpgd.c"
    "${MULTIGAUGE_DEPS_DIR}/tjpgd/tjpgd.c"
  )
  multigauge_download_file(
    "https://raw.githubusercontent.com/cmumford/TJpgDec/master/src/tjpgd.h"
    "${MULTIGAUGE_DEPS_DIR}/tjpgd/tjpgd.h"
  )
  multigauge_download_file(
    "https://raw.githubusercontent.com/cmumford/TJpgDec/master/src/tjpgdcnf.h"
    "${MULTIGAUGE_DEPS_DIR}/tjpgd/tjpgdcnf.h"
  )

  # ESP32-specific dependencies.
  multigauge_clone_repo(
    "https://github.com/me-no-dev/AsyncTCP.git"
    "master"
    "${MULTIGAUGE_DEPS_DIR}/AsyncTCP"
  )
  multigauge_clone_repo(
    "https://github.com/me-no-dev/ESPAsyncWebServer.git"
    "master"
    "${MULTIGAUGE_DEPS_DIR}/ESPAsyncWebServer"
  )
  multigauge_clone_repo(
    "https://github.com/lovyan03/LovyanGFX.git"
    "1.2.0"
    "${MULTIGAUGE_DEPS_DIR}/LovyanGFX"
  )

  # Legacy build-system compatibility directories.
  multigauge_copy_or_link_directory("${MULTIGAUGE_DEPS_DIR}/rapidjson/include" "${MULTIGAUGE_CORE_LIB_DIR}/rapidjson/src")
  multigauge_copy_or_link_directory("${MULTIGAUGE_DEPS_DIR}/yoga/yoga" "${MULTIGAUGE_CORE_LIB_DIR}/yoga/src")
  multigauge_copy_or_link_directory("${MULTIGAUGE_DEPS_DIR}/lodepng" "${MULTIGAUGE_CORE_LIB_DIR}/lodepng/src")
  multigauge_copy_or_link_directory("${MULTIGAUGE_DEPS_DIR}/tjpgd" "${MULTIGAUGE_CORE_LIB_DIR}/tjpgd/src")

  multigauge_copy_or_link_directory("${MULTIGAUGE_DEPS_DIR}/rapidjson/include/rapidjson" "${MULTIGAUGE_SOURCE_DIR}/core/include/rapidjson")
  multigauge_copy_or_link_directory("${MULTIGAUGE_DEPS_DIR}/yoga/yoga" "${MULTIGAUGE_SOURCE_DIR}/core/include/yoga")
  multigauge_copy_file("${MULTIGAUGE_DEPS_DIR}/lodepng/lodepng.h" "${MULTIGAUGE_SOURCE_DIR}/core/include/lodepng.h")
  multigauge_copy_file("${MULTIGAUGE_DEPS_DIR}/tjpgd/tjpgd.h" "${MULTIGAUGE_SOURCE_DIR}/core/include/tjpgd.h")
  multigauge_copy_file("${MULTIGAUGE_DEPS_DIR}/tjpgd/tjpgdcnf.h" "${MULTIGAUGE_SOURCE_DIR}/core/include/tjpgdcnf.h")

  multigauge_copy_or_link_directory("${MULTIGAUGE_DEPS_DIR}/AsyncTCP" "${MULTIGAUGE_ESP32_LIB_DIR}/AsyncTCP-master")
  multigauge_copy_or_link_directory("${MULTIGAUGE_DEPS_DIR}/ESPAsyncWebServer" "${MULTIGAUGE_ESP32_LIB_DIR}/ESPAsyncWebServer-master")
  multigauge_copy_or_link_directory("${MULTIGAUGE_DEPS_DIR}/LovyanGFX" "${MULTIGAUGE_ESP32_LIB_DIR}/LovyanGFX")
  multigauge_copy_or_link_directory("${MULTIGAUGE_SOURCE_DIR}/core" "${MULTIGAUGE_ESP32_LIB_DIR}/multigauge-core")
endfunction()

multigauge_bootstrap_dependencies()
