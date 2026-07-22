include(FetchContent)

FetchContent_Declare(
  doctest
  URL https://github.com/doctest/doctest/archive/refs/tags/v2.4.11.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(doctest)

add_executable(multigauge_core_tests
  test_unit_type.cpp
  test_value.cpp
  test_value_ref.cpp
  test_value_view.cpp
  test_value_embed.cpp
)

target_link_libraries(multigauge_core_tests
  PRIVATE
    multigauge
    doctest::doctest
)

add_test(NAME multigauge_core_tests COMMAND multigauge_core_tests)
