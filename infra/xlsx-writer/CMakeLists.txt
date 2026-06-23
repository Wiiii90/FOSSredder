# Infra XLSX writer adapter.
set(INFRA_XLSX_WRITER_SRCS
    src/XlntTableWriterAdapter.cpp
)

add_library(fossredder_infra_xlsx_writer STATIC ${INFRA_XLSX_WRITER_SRCS})
add_library(FossRedder::InfraXlsxWriter ALIAS fossredder_infra_xlsx_writer)
fossredder_enable_target_quality(fossredder_infra_xlsx_writer)

set_target_properties(fossredder_infra_xlsx_writer PROPERTIES EXPORT_NAME InfraXlsxWriter)

target_include_directories(fossredder_infra_xlsx_writer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(fossredder_infra_xlsx_writer PUBLIC cxx_std_20)

find_package(xlnt CONFIG REQUIRED)

target_link_libraries(fossredder_infra_xlsx_writer
    PUBLIC
        FossRedder::Core
)

if(TARGET xlnt::xlnt)
    target_link_libraries(fossredder_infra_xlsx_writer PRIVATE xlnt::xlnt)
elseif(TARGET xlnt)
    target_link_libraries(fossredder_infra_xlsx_writer PRIVATE xlnt)
else()
    message(FATAL_ERROR "xlnt package was found, but no supported xlnt target is available")
endif()

fossredder_set_library_output_dirs(fossredder_infra_xlsx_writer)

fossredder_install_library_target(fossredder_infra_xlsx_writer)
fossredder_install_headers("${CMAKE_CURRENT_SOURCE_DIR}/include")

option(BUILD_INFRA_XLSX_WRITER_TESTS "Build tests for infra xlsx-writer module" OFF)
if(BUILD_INFRA_XLSX_WRITER_TESTS)
    add_subdirectory(tests)
endif()
