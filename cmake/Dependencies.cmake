include("${CMAKE_CURRENT_LIST_DIR}/MultigaugeDependencies.cmake")

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
endfunction()
