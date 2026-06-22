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
        Tesseract::libtesseract
)

fossredder_set_library_output_dirs(fossredder_infra_text_recognition)

fossredder_install_library_target(fossredder_infra_text_recognition)
fossredder_install_headers("${CMAKE_CURRENT_SOURCE_DIR}/include")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/res/tessdata")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/res/tessdata/"
        DESTINATION "bin/res/tessdata"
        FILES_MATCHING PATTERN "*.traineddata"
    )
endif()

option(BUILD_INFRA_TEXT_RECOGNITION_TESTS "Build tests for infra text-recognition module" OFF)
if(BUILD_INFRA_TEXT_RECOGNITION_TESTS)
    add_subdirectory(tests)
endif()
