include("${CMAKE_CURRENT_LIST_DIR}/../../../core/cmake/Dependencies.cmake")

set(MULTIGAUGE_ESP32_LIB_DIR "${MULTIGAUGE_SOURCE_DIR}/ports/esp32/lib")

function(multigauge_apply_espasyncwebserver_platformio_metadata library_dir)
  set(library_json "${library_dir}/library.json")
  set(override_template "${MULTIGAUGE_SOURCE_DIR}/ports/esp32/platformio-overrides/ESPAsyncWebServer.library.json.in")

  if(NOT EXISTS "${library_json}" OR NOT EXISTS "${override_template}")
    return()
  endif()

  configure_file("${override_template}" "${library_json}" @ONLY)
endfunction()

function(multigauge_bootstrap_esp32_dependencies)
  file(MAKE_DIRECTORY "${MULTIGAUGE_DEPS_DIR}")

  multigauge_clone_dependency(ASYNCTCP)

  multigauge_clone_dependency(ESPASYNCWEBSERVER)
  multigauge_apply_espasyncwebserver_platformio_metadata("${MULTIGAUGE_DEP_ESPASYNCWEBSERVER_DIR}")

  multigauge_clone_dependency(LOVYANGFX)

  multigauge_copy_or_link_directory("${MULTIGAUGE_DEP_ASYNCTCP_DIR}" "${MULTIGAUGE_ESP32_LIB_DIR}/AsyncTCP-master")
  multigauge_copy_or_link_directory("${MULTIGAUGE_DEP_ESPASYNCWEBSERVER_DIR}" "${MULTIGAUGE_ESP32_LIB_DIR}/ESPAsyncWebServer-master")
  multigauge_apply_espasyncwebserver_platformio_metadata("${MULTIGAUGE_ESP32_LIB_DIR}/ESPAsyncWebServer-master")
  multigauge_copy_or_link_directory("${MULTIGAUGE_DEP_LOVYANGFX_DIR}" "${MULTIGAUGE_ESP32_LIB_DIR}/LovyanGFX")
  multigauge_copy_or_link_directory("${MULTIGAUGE_SOURCE_DIR}/core" "${MULTIGAUGE_ESP32_LIB_DIR}/multigauge-core")
endfunction()

function(multigauge_bootstrap_esp32_port_dependencies)
  multigauge_bootstrap_core_dependencies()
  multigauge_bootstrap_esp32_dependencies()
endfunction()
