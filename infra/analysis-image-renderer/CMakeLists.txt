# Infra analysis image renderer adapter.
set(INFRA_ANALYSIS_IMAGE_RENDERER_SRCS
    src/OpenCvAnalysisImageRendererAdapter.cpp
)

add_library(fossredder_infra_analysis_image_renderer STATIC ${INFRA_ANALYSIS_IMAGE_RENDERER_SRCS})
add_library(FossRedder::InfraAnalysisImageRenderer ALIAS fossredder_infra_analysis_image_renderer)
fossredder_enable_target_quality(fossredder_infra_analysis_image_renderer)

set_target_properties(fossredder_infra_analysis_image_renderer PROPERTIES EXPORT_NAME InfraAnalysisImageRenderer)

target_include_directories(fossredder_infra_analysis_image_renderer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(fossredder_infra_analysis_image_renderer PUBLIC cxx_std_20)

find_package(OpenCV REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

target_link_libraries(fossredder_infra_analysis_image_renderer
    PUBLIC
        FossRedder::Core
    PRIVATE
        ${OpenCV_LIBS}
        nlohmann_json::nlohmann_json
)

fossredder_set_library_output_dirs(fossredder_infra_analysis_image_renderer)

fossredder_install_library_target(fossredder_infra_analysis_image_renderer)
fossredder_install_headers("${CMAKE_CURRENT_SOURCE_DIR}/include")

option(BUILD_INFRA_ANALYSIS_IMAGE_RENDERER_TESTS "Build tests for infra analysis-image-renderer module" OFF)
if(BUILD_INFRA_ANALYSIS_IMAGE_RENDERER_TESTS)
    find_package(GTest CONFIG REQUIRED)
    find_package(OpenCV REQUIRED)
    include(GoogleTest)

    add_executable(infra_analysis_image_renderer_unit_tests
        tests/unit/TestOpenCvAnalysisImageRendererAdapter.cpp
    )
    target_compile_features(infra_analysis_image_renderer_unit_tests PRIVATE cxx_std_20)
    target_include_directories(infra_analysis_image_renderer_unit_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
        ${OpenCV_INCLUDE_DIRS}
    )
    target_link_libraries(infra_analysis_image_renderer_unit_tests
        PRIVATE
            GTest::gtest_main
            FossRedder::InfraAnalysisImageRenderer
            ${OpenCV_LIBS}
    )
    fossredder_enable_target_quality(infra_analysis_image_renderer_unit_tests)
    fossredder_set_runtime_output_dirs(infra_analysis_image_renderer_unit_tests)
    gtest_discover_tests(infra_analysis_image_renderer_unit_tests PROPERTIES LABELS "INFRA_ANALYSIS_IMAGE_RENDERER")
endif()
