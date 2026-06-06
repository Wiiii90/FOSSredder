# Infra implementation sources.
set(INFRA_PDF_RENDERING_SRCS
    src/PopplerPdfRendererAdapter.cpp
    src/PopplerCore.cpp
)

add_library(fossredder_infra_pdf_rendering STATIC ${INFRA_PDF_RENDERING_SRCS})
add_library(FossRedder::InfraPdfRendering ALIAS fossredder_infra_pdf_rendering)
fossredder_enable_target_quality(fossredder_infra_pdf_rendering)
fossredder_enable_target_pch(fossredder_infra_pdf_rendering "${CMAKE_CURRENT_SOURCE_DIR}/include/pdf-rendering/pch.h")

set_target_properties(fossredder_infra_pdf_rendering PROPERTIES EXPORT_NAME InfraPdfRendering)

target_include_directories(fossredder_infra_pdf_rendering PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(fossredder_infra_pdf_rendering PUBLIC cxx_std_20)

find_package(PkgConfig REQUIRED)
pkg_check_modules(POPPLER_CPP REQUIRED IMPORTED_TARGET poppler-cpp)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenCV REQUIRED)

target_link_libraries(fossredder_infra_pdf_rendering
    PRIVATE
        FossRedder::Core
        FossRedder::Debug
        PkgConfig::POPPLER_CPP
        nlohmann_json::nlohmann_json
        ${OpenCV_LIBS}
)

fossredder_set_library_output_dirs(fossredder_infra_pdf_rendering)

fossredder_install_library_target(fossredder_infra_pdf_rendering)
fossredder_install_headers("${CMAKE_CURRENT_SOURCE_DIR}/include")

option(BUILD_INFRA_PDF_RENDERING_TESTS "Build tests for infra pdf-rendering module" OFF)
if(BUILD_INFRA_PDF_RENDERING_TESTS)
    find_package(GTest CONFIG REQUIRED)
    include(GoogleTest)

    add_executable(infra_pdf_rendering_unit_tests
        tests/unit/TestPopplerCore.cpp
        tests/unit/TestPopplerPdfRendererAdapter.cpp
    )
    target_compile_features(infra_pdf_rendering_unit_tests PRIVATE cxx_std_20)
    target_include_directories(infra_pdf_rendering_unit_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/core/include
    )
    target_link_libraries(infra_pdf_rendering_unit_tests
        PRIVATE
            GTest::gtest_main
            FossRedder::InfraPdfRendering
    )
    fossredder_enable_target_quality(infra_pdf_rendering_unit_tests)
    fossredder_set_runtime_output_dirs(infra_pdf_rendering_unit_tests)
    gtest_discover_tests(infra_pdf_rendering_unit_tests PROPERTIES LABELS "INFRA_PDF_RENDERING")
endif()
