set(INFRA_ARCHIVE_SRCS
    src/ZipArchiveAdapter.cpp
)

add_library(fossredder_infra_archive STATIC ${INFRA_ARCHIVE_SRCS})
add_library(FossRedder::InfraArchive ALIAS fossredder_infra_archive)
fossredder_enable_target_quality(fossredder_infra_archive)
fossredder_enable_target_pch(fossredder_infra_archive "${CMAKE_CURRENT_SOURCE_DIR}/include/archive/pch.h")

set_target_properties(fossredder_infra_archive PROPERTIES EXPORT_NAME InfraArchive)

target_include_directories(fossredder_infra_archive PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(fossredder_infra_archive PUBLIC cxx_std_20)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBZIP REQUIRED IMPORTED_TARGET libzip)

target_link_libraries(fossredder_infra_archive
    PUBLIC
        FossRedder::Core
    PRIVATE
        PkgConfig::LIBZIP
)

fossredder_set_library_output_dirs(fossredder_infra_archive)

fossredder_install_library_target(fossredder_infra_archive)
fossredder_install_headers("${CMAKE_CURRENT_SOURCE_DIR}/include")

option(BUILD_INFRA_ARCHIVE_TESTS "Build tests for infra archive module" OFF)
if(BUILD_INFRA_ARCHIVE_TESTS)
    find_package(GTest CONFIG REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBZIP REQUIRED IMPORTED_TARGET libzip)
    include(GoogleTest)

    add_executable(infra_archive_unit_tests
        tests/unit/TestZipArchiveAdapter.cpp
    )
    target_compile_features(infra_archive_unit_tests PRIVATE cxx_std_20)
    target_include_directories(infra_archive_unit_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )
    target_link_libraries(infra_archive_unit_tests
        PRIVATE
            GTest::gtest_main
            FossRedder::InfraArchive
            PkgConfig::LIBZIP
    )
    fossredder_enable_target_quality(infra_archive_unit_tests)
    fossredder_set_runtime_output_dirs(infra_archive_unit_tests)
    gtest_discover_tests(infra_archive_unit_tests PROPERTIES LABELS "INFRA_ARCHIVE")
endif()
