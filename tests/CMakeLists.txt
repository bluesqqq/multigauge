add_executable(multigauge_core_tests
  test_main.cpp
  test_json.cpp
  test_unit_type.cpp
  test_value.cpp
  test_value_ref.cpp
  test_value_view.cpp
  test_value_embed.cpp
  test_properties.cpp
  test_polymorphic_registry.cpp
  test_gauge.cpp
  test_colors.cpp
  test_sensors.cpp
)

target_include_directories(multigauge_core_tests PRIVATE
  "${MULTIGAUGE_DOCTEST_INCLUDE_DIR}"
)

target_link_libraries(multigauge_core_tests
  PRIVATE
    multigauge
)

add_test(NAME multigauge_core_tests COMMAND multigauge_core_tests)
