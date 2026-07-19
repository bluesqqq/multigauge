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

# Single source of truth for third-party dependency locations and versions.
set(MULTIGAUGE_DEP_RAPIDJSON_URL "https://github.com/Tencent/rapidjson.git")
set(MULTIGAUGE_DEP_RAPIDJSON_TAG "v1.1.0")
set(MULTIGAUGE_DEP_RAPIDJSON_DIR "${MULTIGAUGE_DEPS_DIR}/rapidjson")

set(MULTIGAUGE_DEP_YOGA_URL "https://github.com/facebook/yoga.git")
set(MULTIGAUGE_DEP_YOGA_TAG "v2.0.0")
set(MULTIGAUGE_DEP_YOGA_DIR "${MULTIGAUGE_DEPS_DIR}/yoga")

set(MULTIGAUGE_DEP_LODEPNG_DIR "${MULTIGAUGE_DEPS_DIR}/lodepng")
set(MULTIGAUGE_DEP_LODEPNG_CPP_URL "https://raw.githubusercontent.com/lvandeve/lodepng/master/lodepng.cpp")
set(MULTIGAUGE_DEP_LODEPNG_H_URL "https://raw.githubusercontent.com/lvandeve/lodepng/master/lodepng.h")

set(MULTIGAUGE_DEP_TJPGD_DIR "${MULTIGAUGE_DEPS_DIR}/tjpgd")
set(MULTIGAUGE_DEP_TJPGD_C_URL "https://raw.githubusercontent.com/cmumford/TJpgDec/master/src/tjpgd.c")
set(MULTIGAUGE_DEP_TJPGD_H_URL "https://raw.githubusercontent.com/cmumford/TJpgDec/master/src/tjpgd.h")
set(MULTIGAUGE_DEP_TJPGD_CONFIG_URL "https://raw.githubusercontent.com/cmumford/TJpgDec/master/src/tjpgdcnf.h")

set(MULTIGAUGE_DEP_ASYNCTCP_URL "https://github.com/me-no-dev/AsyncTCP.git")
set(MULTIGAUGE_DEP_ASYNCTCP_TAG "master")
set(MULTIGAUGE_DEP_ASYNCTCP_DIR "${MULTIGAUGE_DEPS_DIR}/AsyncTCP")

set(MULTIGAUGE_DEP_ESPASYNCWEBSERVER_URL "https://github.com/me-no-dev/ESPAsyncWebServer.git")
set(MULTIGAUGE_DEP_ESPASYNCWEBSERVER_TAG "master")
set(MULTIGAUGE_DEP_ESPASYNCWEBSERVER_VERSION "3.6.0")
set(MULTIGAUGE_DEP_ESPASYNCWEBSERVER_DIR "${MULTIGAUGE_DEPS_DIR}/ESPAsyncWebServer")

set(MULTIGAUGE_DEP_LOVYANGFX_URL "https://github.com/lovyan03/LovyanGFX.git")
set(MULTIGAUGE_DEP_LOVYANGFX_TAG "1.2.25")
set(MULTIGAUGE_DEP_LOVYANGFX_DIR "${MULTIGAUGE_DEPS_DIR}/LovyanGFX")

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

function(multigauge_clone_dependency name)
  set(url_var "MULTIGAUGE_DEP_${name}_URL")
  set(tag_var "MULTIGAUGE_DEP_${name}_TAG")
  set(dir_var "MULTIGAUGE_DEP_${name}_DIR")

  if(NOT DEFINED ${url_var} OR NOT DEFINED ${tag_var} OR NOT DEFINED ${dir_var})
    message(FATAL_ERROR "Unknown multigauge dependency '${name}'")
  endif()

  multigauge_clone_repo("${${url_var}}" "${${tag_var}}" "${${dir_var}}")
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
