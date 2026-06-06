include_guard(GLOBAL)

function(fossredder_configure_packaging)
    if(NOT WIN32)
        return()
    endif()

    set(_package_version "${PROJECT_VERSION}")
    if(_package_version STREQUAL "")
        set(_package_version "0.1.0")
    endif()

    set(_staging_dir "${CMAKE_BINARY_DIR}/staging")
    set(_output_dir "${CMAKE_BINARY_DIR}/dist")
    set(_powershell_exe "C:/Windows/System32/WindowsPowerShell/v1.0/powershell.exe")
    set(_package_log "${CMAKE_BINARY_DIR}/package.log")
    set(_run_package_cmd "${CMAKE_BINARY_DIR}/run-package.cmd")

    set(BUILD_DIR "${CMAKE_BINARY_DIR}")
    set(PACKAGE_SCRIPT "${CMAKE_SOURCE_DIR}/ci/package/package-inno.ps1")
    set(STAGING_DIR "${_staging_dir}")
    set(OUTPUT_DIR "${_output_dir}")
    set(POWERSHELL_EXE "${_powershell_exe}")
    set(VERSION "${_package_version}")
    set(LOG_FILE "${_package_log}")

    configure_file(
        "${CMAKE_SOURCE_DIR}/cmake/templates/run-package.cmd.in"
        "${_run_package_cmd}"
        @ONLY
    )

    add_custom_target(package
        COMMENT "Install and create Inno Setup installer"
        COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --config $<CONFIG> --prefix "${_staging_dir}"
        COMMAND ${CMAKE_COMMAND} -E echo "Running packaging wrapper (log: ${_package_log})..."
        COMMAND cmd.exe /c call "${_run_package_cmd}" $<CONFIG>
        DEPENDS fossredder
        BYPRODUCTS "${_staging_dir}" "${_output_dir}" "${_package_log}" "${_run_package_cmd}"
        VERBATIM
    )
endfunction()
