find_package(GTest CONFIG REQUIRED)
include(GoogleTest)

add_executable(document_image_processing_unit_tests
    unit/TestCropAdapter.cpp
    unit/TestDetectAdapter.cpp
    unit/TestDenoiseAdapter.cpp
    unit/TestMaskAdapter.cpp
)
target_compile_features(document_image_processing_unit_tests PRIVATE cxx_std_20)
target_include_directories(document_image_processing_unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(document_image_processing_unit_tests
    PRIVATE
        GTest::gtest_main
        FossRedder::InfraDocumentImageProcessing
)
fossredder_set_runtime_output_dirs(document_image_processing_unit_tests)
fossredder_enable_target_quality(document_image_processing_unit_tests)
gtest_discover_tests(document_image_processing_unit_tests PROPERTIES LABELS "DOCUMENT_IMAGE_PROCESSING_UNIT")
