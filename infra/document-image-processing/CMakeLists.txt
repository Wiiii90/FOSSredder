# Infra implementation sources.
set(INFRA_DOCUMENT_IMAGE_PROCESSING_SRCS
    src/CropAdapter.cpp
    src/DenoiseAdapter.cpp
    src/DetectAdapter.cpp
    src/MaskAdapter.cpp
    src/OpenCvDocumentImageProcessorAdapter.cpp
)

add_library(fossredder_infra_document_image_processing STATIC ${INFRA_DOCUMENT_IMAGE_PROCESSING_SRCS})
add_library(FossRedder::InfraDocumentImageProcessing ALIAS fossredder_infra_document_image_processing)
fossredder_enable_target_quality(fossredder_infra_document_image_processing)
fossredder_enable_target_pch(fossredder_infra_document_image_processing "${CMAKE_CURRENT_SOURCE_DIR}/include/document-image-processing/pch.h")

set_target_properties(fossredder_infra_document_image_processing PROPERTIES EXPORT_NAME InfraDocumentImageProcessing)

target_include_directories(fossredder_infra_document_image_processing PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(fossredder_infra_document_image_processing PUBLIC cxx_std_20)

# OpenCV types are part of the public port headers.
find_package(OpenCV REQUIRED)
target_link_libraries(fossredder_infra_document_image_processing
    PUBLIC
        FossRedder::Core
        ${OpenCV_LIBS}
)

fossredder_set_library_output_dirs(fossredder_infra_document_image_processing)

fossredder_install_library_target(fossredder_infra_document_image_processing)
fossredder_install_headers("${CMAKE_CURRENT_SOURCE_DIR}/include")

# Optional module-local tests.
option(BUILD_INFRA_DOCUMENT_IMAGE_PROCESSING_TESTS "Build tests for document image processing infrastructure" OFF)
if(BUILD_INFRA_DOCUMENT_IMAGE_PROCESSING_TESTS)
    add_subdirectory(tests)
endif()
