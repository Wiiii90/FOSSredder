# Infra implementation sources.
set(INFRA_TEXT_RECOGNITION_SRCS
    src/TesseractTextRecognizerAdapter.cpp
    src/TesseractCore.cpp
)

add_library(fossredder_infra_text_recognition STATIC ${INFRA_TEXT_RECOGNITION_SRCS})
add_library(FossRedder::InfraTextRecognition ALIAS fossredder_infra_text_recognition)
fossredder_enable_target_quality(fossredder_infra_text_recognition)
fossredder_enable_target_pch(fossredder_infra_text_recognition "${CMAKE_CURRENT_SOURCE_DIR}/include/text-recognition/pch.h")

set_target_properties(fossredder_infra_text_recognition PROPERTIES EXPORT_NAME InfraTextRecognition)

target_include_directories(fossredder_infra_text_recognition PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(fossredder_infra_text_recognition PUBLIC cxx_std_20)

find_package(Tesseract REQUIRED CONFIG)
target_link_libraries(fossredder_infra_text_recognition
    PRIVATE
        FossRedder::Core
        FossRedder::Debug
        Tesseract::libtesseract
)

fossredder_set_library_output_dirs(fossredder_infra_text_recognition)

fossredder_install_library_target(fossredder_infra_text_recognition)
fossredder_install_headers("${CMAKE_CURRENT_SOURCE_DIR}/include")

option(BUILD_INFRA_TEXT_RECOGNITION_TESTS "Build tests for infra text-recognition module" OFF)
if(BUILD_INFRA_TEXT_RECOGNITION_TESTS)
    find_package(GTest CONFIG REQUIRED)
    find_package(OpenCV REQUIRED)
    include(GoogleTest)

    add_executable(infra_text_recognition_unit_tests
        tests/unit/TestTesseractCore.cpp
        tests/unit/TestTesseractTextRecognizerAdapter.cpp
    )
    target_compile_features(infra_text_recognition_unit_tests PRIVATE cxx_std_20)
    target_include_directories(infra_text_recognition_unit_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
        ${CMAKE_CURRENT_SOURCE_DIR}/include
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
    gtest_discover_tests(infra_text_recognition_unit_tests PROPERTIES LABELS "INFRA_TEXT_RECOGNITION")
endif()
