include("${CMAKE_CURRENT_LIST_DIR}/../../cmake/MultigaugeDependencies.cmake")

set(MULTIGAUGE_CORE_LIB_DIR "${MULTIGAUGE_SOURCE_DIR}/core/lib")

function(multigauge_bootstrap_core_dependencies)
  file(MAKE_DIRECTORY "${MULTIGAUGE_DEPS_DIR}")

  multigauge_clone_dependency(RAPIDJSON)
  multigauge_patch_rapidjson("${MULTIGAUGE_DEP_RAPIDJSON_DIR}/include")

  multigauge_clone_dependency(YOGA)

  multigauge_download_file(
    "${MULTIGAUGE_DEP_LODEPNG_CPP_URL}"
    "${MULTIGAUGE_DEP_LODEPNG_DIR}/lodepng.cpp"
  )
  multigauge_download_file(
    "${MULTIGAUGE_DEP_LODEPNG_H_URL}"
    "${MULTIGAUGE_DEP_LODEPNG_DIR}/lodepng.h"
  )

  multigauge_download_file(
    "${MULTIGAUGE_DEP_TJPGD_C_URL}"
    "${MULTIGAUGE_DEP_TJPGD_DIR}/tjpgd.c"
  )
  multigauge_download_file(
    "${MULTIGAUGE_DEP_TJPGD_H_URL}"
    "${MULTIGAUGE_DEP_TJPGD_DIR}/tjpgd.h"
  )
  multigauge_download_file(
    "${MULTIGAUGE_DEP_TJPGD_CONFIG_URL}"
    "${MULTIGAUGE_DEP_TJPGD_DIR}/tjpgdcnf.h"
  )

  multigauge_copy_or_link_directory("${MULTIGAUGE_DEP_RAPIDJSON_DIR}/include" "${MULTIGAUGE_CORE_LIB_DIR}/rapidjson/src")
  multigauge_copy_or_link_directory("${MULTIGAUGE_DEP_YOGA_DIR}/yoga" "${MULTIGAUGE_CORE_LIB_DIR}/yoga/src")
  multigauge_copy_or_link_directory("${MULTIGAUGE_DEP_LODEPNG_DIR}" "${MULTIGAUGE_CORE_LIB_DIR}/lodepng/src")
  multigauge_copy_or_link_directory("${MULTIGAUGE_DEP_TJPGD_DIR}" "${MULTIGAUGE_CORE_LIB_DIR}/tjpgd/src")

  multigauge_copy_or_link_directory("${MULTIGAUGE_DEP_RAPIDJSON_DIR}/include/rapidjson" "${MULTIGAUGE_SOURCE_DIR}/core/include/rapidjson")
  multigauge_copy_or_link_directory("${MULTIGAUGE_DEP_YOGA_DIR}/yoga" "${MULTIGAUGE_SOURCE_DIR}/core/include/yoga")
  multigauge_copy_file("${MULTIGAUGE_DEP_LODEPNG_DIR}/lodepng.h" "${MULTIGAUGE_SOURCE_DIR}/core/include/lodepng.h")
  multigauge_copy_file("${MULTIGAUGE_DEP_TJPGD_DIR}/tjpgd.h" "${MULTIGAUGE_SOURCE_DIR}/core/include/tjpgd.h")
  multigauge_copy_file("${MULTIGAUGE_DEP_TJPGD_DIR}/tjpgdcnf.h" "${MULTIGAUGE_SOURCE_DIR}/core/include/tjpgdcnf.h")
endfunction()
