cmake_minimum_required(VERSION 4.0.1)

include(cmake/Vcpkg.cmake)
fossredder_configure_vcpkg()

project(FossRedder VERSION 0.1.0 LANGUAGES CXX)

include(CMakePackageConfigHelpers)
include(CTest)
include(cmake/Targets.cmake)
include(cmake/QualityTools.cmake)

fossredder_configure_quality_tools()

# Global C++ configuration.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
if(MSVC)
    # MSVC needs /FS when multiple generated sources can write the same PDB in parallel.
    add_compile_options(/FS)
endif()

# Generate Qt Creator/QML Language Server metadata for QML editing.
set(QT_QML_GENERATE_QMLLS_INI ON)

# Module layout.
add_subdirectory(core)
add_subdirectory(debug)
add_subdirectory(infra)
add_subdirectory(persistence)
add_subdirectory(ui)
add_subdirectory(app)

set(FOSSREDDER_QT_PUBLIC_COMPONENTS)
if(TARGET Qt6::Quick AND TARGET Qt6::Qml)
    list(APPEND FOSSREDDER_QT_PUBLIC_COMPONENTS Quick Qml)
endif()
if(TARGET Qt6::QuickControls2)
    list(APPEND FOSSREDDER_QT_PUBLIC_COMPONENTS QuickControls2)
endif()
if(TARGET Qt6::Widgets)
    list(APPEND FOSSREDDER_QT_PUBLIC_COMPONENTS Widgets)
endif()
if(TARGET Qt6::Svg)
    list(APPEND FOSSREDDER_QT_PUBLIC_COMPONENTS Svg)
endif()
if(TARGET Qt6::Concurrent)
    list(APPEND FOSSREDDER_QT_PUBLIC_COMPONENTS Concurrent)
endif()
list(REMOVE_DUPLICATES FOSSREDDER_QT_PUBLIC_COMPONENTS)

configure_package_config_file(
    "${CMAKE_SOURCE_DIR}/cmake/FossRedderConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/FossRedderConfig.cmake"
    INSTALL_DESTINATION "${FOSSREDDER_INSTALL_CMAKEDIR}"
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/FossRedderConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(EXPORT ${FOSSREDDER_EXPORT_SET}
    FILE FossRedderTargets.cmake
    NAMESPACE FossRedder::
    DESTINATION "${FOSSREDDER_INSTALL_CMAKEDIR}"
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/FossRedderConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/FossRedderConfigVersion.cmake"
    DESTINATION "${FOSSREDDER_INSTALL_CMAKEDIR}"
)

include(cmake/Packaging.cmake)
fossredder_configure_packaging()
