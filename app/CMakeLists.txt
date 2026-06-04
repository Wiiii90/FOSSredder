# UI startup options.
option(USE_QML "Build with QML UI" OFF)

set(APP_COMMON_SRCS
    src/Environment.cpp
    src/main.cpp
)

set(APP_QML_SRCS
    src/main_qml.cpp
)

set(APP_RESOURCE_SRCS
    assets/fossredder_resources.qrc
)

if(WIN32)
    set(APP_ICON_ICO "${CMAKE_CURRENT_SOURCE_DIR}/assets/icons/fossredder.ico")
    set(APP_ICON_RC "${CMAKE_CURRENT_BINARY_DIR}/fossredder_icon.rc")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/fossredder_icon.rc.in"
        "${APP_ICON_RC}"
        @ONLY
    )
    list(APPEND APP_RESOURCE_SRCS "${APP_ICON_RC}")
endif()

find_package(Qt6 COMPONENTS Quick Qml Widgets LinguistTools QUIET)
if(TARGET Qt6::Quick AND TARGET Qt6::Qml)
    if(NOT USE_QML)
        set(USE_QML ON CACHE BOOL "Build with QML UI" FORCE)
    endif()
else()
    if(USE_QML)
        message(WARNING "USE_QML is ON but Qt Quick/Qml not found; build may fail or QML will be unavailable.")
    endif()
endif()

set(APP_SRCS ${APP_COMMON_SRCS} ${APP_RESOURCE_SRCS})
if(USE_QML)
    list(APPEND APP_SRCS ${APP_QML_SRCS})
endif()

add_executable(fossredder ${APP_SRCS})
fossredder_enable_target_quality(fossredder)

# Enable Qt code generation on the application target only.
set_target_properties(fossredder PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

target_compile_features(fossredder PUBLIC cxx_std_20)

# Application dependencies.
target_link_libraries(fossredder PRIVATE
    FossRedder::Core
    FossRedder::Debug
    FossRedder::Persistence
    FossRedder::InfraAnalysisImageRenderer
    FossRedder::InfraImageProcessing
    FossRedder::InfraArchive
    FossRedder::InfraPdfRendering
    FossRedder::InfraXlsxWriter
    FossRedder::InfraTextRecognition
    FossRedder::Ui
)

if(MSVC)
    target_compile_options(fossredder PRIVATE /Zc:__cplusplus)

    if (WIN32)
        target_link_options(fossredder PRIVATE
            "$<$<CONFIG:Debug>:/SUBSYSTEM:CONSOLE>"
            "$<$<NOT:$<CONFIG:Debug>>:/SUBSYSTEM:WINDOWS>"
            "$<$<NOT:$<CONFIG:Debug>>:/ENTRY:mainCRTStartup>"
        )
    endif()
endif()

fossredder_set_runtime_output_dirs(fossredder)

target_include_directories(fossredder PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Translation sources.
set(TRANSLATION_TS_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/i18n/fossredder_de.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/i18n/fossredder_fr.ts
)

set(TRANSLATION_SOURCE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/ui/src
    ${CMAKE_SOURCE_DIR}/ui/include
    ${CMAKE_SOURCE_DIR}/ui/qml
)

# Translation generation helpers.
if(TARGET Qt6::lupdate AND TARGET Qt6::lrelease)
    set(TRANSLATION_QM_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/i18n)
    set(TRANSLATION_QM_FILES)

    foreach(TS_FILE IN LISTS TRANSLATION_TS_FILES)
        get_filename_component(TS_NAME ${TS_FILE} NAME_WE)
        set(QM_FILE ${TRANSLATION_QM_OUTPUT_DIR}/${TS_NAME}.qm)

        list(APPEND TRANSLATION_QM_FILES ${QM_FILE})

        add_custom_command(
            OUTPUT ${QM_FILE}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${TRANSLATION_QM_OUTPUT_DIR}
            COMMAND Qt6::lrelease ${TS_FILE} -qm ${QM_FILE}
            DEPENDS ${TS_FILE}
            VERBATIM
        )
    endforeach()

    add_custom_target(fossredder_translations DEPENDS ${TRANSLATION_QM_FILES})
    add_dependencies(fossredder fossredder_translations)

    add_custom_command(TARGET fossredder POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:fossredder>/i18n"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TRANSLATION_QM_FILES} "$<TARGET_FILE_DIR:fossredder>/i18n"
        VERBATIM
    )

    add_custom_target(update_translations)
    foreach(TS_FILE IN LISTS TRANSLATION_TS_FILES)
        add_custom_command(TARGET update_translations POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${TRANSLATION_QM_OUTPUT_DIR}
            COMMAND Qt6::lupdate ${TRANSLATION_SOURCE_DIRS} -no-obsolete -ts ${TS_FILE}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            VERBATIM
        )
    endforeach()
else()
    message(WARNING "Qt6 LinguistTools not found; translation catalogs will not be generated automatically")
endif()

# UI toolkit linkage.
if(TARGET Qt6::Quick AND TARGET Qt6::Qml)
    target_link_libraries(fossredder PRIVATE Qt6::Quick Qt6::Qml)
    target_compile_definitions(fossredder PRIVATE USE_QML=1)
endif()

if(TARGET Qt6::Widgets)
    target_link_libraries(fossredder PRIVATE Qt6::Widgets)
endif()

fossredder_enable_target_pch(fossredder "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h")

fossredder_configure_qt_runtime(fossredder
    QML_SOURCE_DIR "${CMAKE_SOURCE_DIR}/ui/qml/FossRedder"
    QML_TOOLING_DIR "${CMAKE_BINARY_DIR}/ui/qmltooling/FossRedder"
)

# Install application runtime assets.
install(TARGETS fossredder
    RUNTIME DESTINATION bin
)

if(EXISTS "${CMAKE_SOURCE_DIR}/ui/qml")
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/ui/qml/" DESTINATION "bin/qml")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/i18n")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/i18n/" DESTINATION "bin/i18n")
endif()

if(TRANSLATION_QM_FILES)
    install(FILES ${TRANSLATION_QM_FILES} DESTINATION "bin/i18n")
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets/" DESTINATION share/fossredder/assets FILES_MATCHING PATTERN "*")
endif()
