# Infra implementation sources.
set(INFRA_IMAGE_PROCESSING_SRCS
    src/CropAdapter.cpp
    src/DenoiseAdapter.cpp
    src/DetectAdapter.cpp
    src/MaskAdapter.cpp
    src/OpenCvImageProcessorAdapter.cpp
)

add_library(fossredder_infra_image_processing STATIC ${INFRA_IMAGE_PROCESSING_SRCS})
add_library(FossRedder::InfraImageProcessing ALIAS fossredder_infra_image_processing)
fossredder_enable_target_quality(fossredder_infra_image_processing)
fossredder_enable_target_pch(fossredder_infra_image_processing "${CMAKE_CURRENT_SOURCE_DIR}/include/image-processing/pch.h")

set_target_properties(fossredder_infra_image_processing PROPERTIES EXPORT_NAME InfraImageProcessing)

target_include_directories(fossredder_infra_image_processing PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(fossredder_infra_image_processing PUBLIC cxx_std_20)

# OpenCV types are part of the public port headers.
find_package(OpenCV REQUIRED)
target_link_libraries(fossredder_infra_image_processing
    PUBLIC
        FossRedder::Core
        ${OpenCV_LIBS}
    PRIVATE
        FossRedder::Debug
)

fossredder_set_library_output_dirs(fossredder_infra_image_processing)

fossredder_install_library_target(fossredder_infra_image_processing)
fossredder_install_headers("${CMAKE_CURRENT_SOURCE_DIR}/include")

# Optional module-local tests.
option(BUILD_OPENCV_TESTS "Build tests for opencv infrastructure" OFF)
if(BUILD_OPENCV_TESTS)
    find_package(GTest CONFIG REQUIRED)
    include(GoogleTest)

    add_executable(opencv_unit_tests
        tests/unit/TestCropAdapter.cpp
        tests/unit/TestDetectAdapter.cpp
        tests/unit/TestDenoiseAdapter.cpp
        tests/unit/TestMaskAdapter.cpp
    )
    target_compile_features(opencv_unit_tests PRIVATE cxx_std_20)
    target_include_directories(opencv_unit_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )
    target_link_libraries(opencv_unit_tests
        PRIVATE
            GTest::gtest_main
            FossRedder::InfraImageProcessing
    )
    fossredder_set_runtime_output_dirs(opencv_unit_tests)
    fossredder_enable_target_quality(opencv_unit_tests)
    gtest_discover_tests(opencv_unit_tests PROPERTIES LABELS "OPENCV_UNIT")
endif()
