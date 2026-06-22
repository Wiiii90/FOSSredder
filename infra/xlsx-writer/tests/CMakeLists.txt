find_package(GTest CONFIG REQUIRED)
find_package(xlnt CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBZIP REQUIRED IMPORTED_TARGET libzip)
include(GoogleTest)

add_executable(infra_xlsx_writer_unit_tests
    unit/TestXlntTableWriterAdapter.cpp
)
target_compile_features(infra_xlsx_writer_unit_tests PRIVATE cxx_std_20)
target_include_directories(infra_xlsx_writer_unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(infra_xlsx_writer_unit_tests
    PRIVATE
        GTest::gtest_main
        FossRedder::InfraXlsxWriter
        xlnt::xlnt
        PkgConfig::LIBZIP
)
fossredder_enable_target_quality(infra_xlsx_writer_unit_tests)
fossredder_set_runtime_output_dirs(infra_xlsx_writer_unit_tests)
gtest_discover_tests(infra_xlsx_writer_unit_tests PROPERTIES LABELS "INFRA_XLSX_WRITER")
