find_package(GTest CONFIG REQUIRED)
find_package(OpenCV REQUIRED)
include(GoogleTest)

add_executable(infra_analysis_rendering_unit_tests
    unit/TestOpenCvAnalysisRendererAdapter.cpp
)
target_compile_features(infra_analysis_rendering_unit_tests PRIVATE cxx_std_20)
target_include_directories(infra_analysis_rendering_unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(infra_analysis_rendering_unit_tests
    PRIVATE
        GTest::gtest_main
        FossRedder::InfraAnalysisRendering
        ${OpenCV_LIBS}
)
fossredder_enable_target_quality(infra_analysis_rendering_unit_tests)
fossredder_set_runtime_output_dirs(infra_analysis_rendering_unit_tests)
gtest_discover_tests(infra_analysis_rendering_unit_tests PROPERTIES LABELS "INFRA_ANALYSIS_RENDERING")
