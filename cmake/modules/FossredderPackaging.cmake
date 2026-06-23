include_guard(GLOBAL)

function(fossredder_configure_packaging)
    if(NOT WIN32)
        return()
    endif()

    set(_package_version "${PROJECT_VERSION}")
    if(_package_version STREQUAL "")
        set(_package_version "0.5.0")
    endif()

    set(_staging_dir "${CMAKE_BINARY_DIR}/staging")
    set(_output_dir "${CMAKE_BINARY_DIR}/dist")
    set(_powershell_exe "C:/Windows/System32/WindowsPowerShell/v1.0/powershell.exe")

    add_custom_target(package
        COMMENT "Install and create Inno Setup installer"
        COMMAND "${_powershell_exe}"
            -NoProfile
            -ExecutionPolicy Bypass
            -File "${CMAKE_SOURCE_DIR}/ci/package/package-inno.ps1"
            -BuildDir "${CMAKE_BINARY_DIR}"
            -Config $<CONFIG>
            -StagingDir "${_staging_dir}"
            -OutputDir "${_output_dir}"
            -Version "${_package_version}"
        DEPENDS fossredder
        BYPRODUCTS "${_staging_dir}" "${_output_dir}"
        VERBATIM
    )
endfunction()
