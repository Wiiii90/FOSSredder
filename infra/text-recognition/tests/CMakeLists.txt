find_package(GTest CONFIG REQUIRED)
find_package(OpenCV REQUIRED)
include(GoogleTest)

add_executable(infra_text_recognition_unit_tests
    unit/TestTesseractCore.cpp
    unit/TestTesseractTextRecognizerAdapter.cpp
)
target_compile_features(infra_text_recognition_unit_tests PRIVATE cxx_std_20)
target_include_directories(infra_text_recognition_unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_SOURCE_DIR}/core/include
    ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(infra_text_recognition_unit_tests
    PRIVATE
        GTest::gtest_main
        FossRedder::InfraTextRecognition
        ${OpenCV_LIBS}
)
fossredder_enable_target_quality(infra_text_recognition_unit_tests)
fossredder_set_runtime_output_dirs(infra_text_recognition_unit_tests)
gtest_discover_tests(infra_text_recognition_unit_tests PROPERTIES LABELS "INFRA_TEXT_RECOGNITION_UNIT")
