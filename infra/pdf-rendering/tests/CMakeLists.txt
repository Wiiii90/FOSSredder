find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

add_executable(infra_pdf_rendering_unit_tests
    unit/TestPopplerCore.cpp
    unit/TestPopplerPdfRendererAdapter.cpp
)
target_compile_features(infra_pdf_rendering_unit_tests PRIVATE cxx_std_20)
target_include_directories(infra_pdf_rendering_unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_SOURCE_DIR}/core/include
)
target_link_libraries(infra_pdf_rendering_unit_tests
    PRIVATE
        GTest::gtest_main
        FossRedder::InfraPdfRendering
)
fossredder_enable_target_quality(infra_pdf_rendering_unit_tests)
fossredder_set_runtime_output_dirs(infra_pdf_rendering_unit_tests)
gtest_discover_tests(infra_pdf_rendering_unit_tests PROPERTIES LABELS "INFRA_PDF_RENDERING_UNIT")
