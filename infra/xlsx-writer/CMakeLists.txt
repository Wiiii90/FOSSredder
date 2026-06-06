# Infra XLSX writer adapter.
set(INFRA_XLSX_WRITER_SRCS
    src/XlntTableWriterAdapter.cpp
)

add_library(fossredder_infra_xlsx_writer STATIC ${INFRA_XLSX_WRITER_SRCS})
add_library(FossRedder::InfraXlsxWriter ALIAS fossredder_infra_xlsx_writer)
fossredder_enable_target_quality(fossredder_infra_xlsx_writer)

set_target_properties(fossredder_infra_xlsx_writer PROPERTIES EXPORT_NAME InfraXlsxWriter)

target_include_directories(fossredder_infra_xlsx_writer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(fossredder_infra_xlsx_writer PUBLIC cxx_std_20)

find_package(xlnt CONFIG QUIET)

target_link_libraries(fossredder_infra_xlsx_writer
    PUBLIC
        FossRedder::Core
)

if(TARGET xlnt::xlnt)
    target_link_libraries(fossredder_infra_xlsx_writer PRIVATE xlnt::xlnt)
elseif(TARGET xlnt)
    target_link_libraries(fossredder_infra_xlsx_writer PRIVATE xlnt)
else()
    message(WARNING "xlnt library not found; XLSX export may fail to link")
endif()

fossredder_set_library_output_dirs(fossredder_infra_xlsx_writer)

fossredder_install_library_target(fossredder_infra_xlsx_writer)
fossredder_install_headers("${CMAKE_CURRENT_SOURCE_DIR}/include")

option(BUILD_INFRA_XLSX_WRITER_TESTS "Build tests for infra xlsx-writer module" OFF)
if(BUILD_INFRA_XLSX_WRITER_TESTS)
    find_package(GTest CONFIG REQUIRED)
    find_package(xlnt CONFIG REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBZIP REQUIRED IMPORTED_TARGET libzip)
    include(GoogleTest)

    add_executable(infra_xlsx_writer_unit_tests
        tests/unit/TestXlntTableWriterAdapter.cpp
    )
    target_compile_features(infra_xlsx_writer_unit_tests PRIVATE cxx_std_20)
    target_include_directories(infra_xlsx_writer_unit_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
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
endif()
