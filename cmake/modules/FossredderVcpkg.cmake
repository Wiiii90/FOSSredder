include_guard(GLOBAL)

function(fossredder_read_windows_env_registry _name _out_var)
    if(NOT WIN32)
        set(${_out_var} "" PARENT_SCOPE)
        return()
    endif()

    execute_process(
        COMMAND reg.exe query "HKCU\\Environment" /v "${_name}"
        OUTPUT_VARIABLE _registry_output
        RESULT_VARIABLE _registry_result
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    if(_registry_result EQUAL 0)
        string(REGEX REPLACE ".*[\r\n][ \t]*${_name}[ \t]+REG_[A-Z0-9_]+[ \t]+([^\r\n]+).*" "\\1" _registry_value "${_registry_output}")
        if(NOT _registry_value STREQUAL "${_registry_output}")
            string(STRIP "${_registry_value}" _registry_value)
            file(TO_CMAKE_PATH "${_registry_value}" _registry_value)
            set(${_out_var} "${_registry_value}" PARENT_SCOPE)
            return()
        endif()
    endif()

    set(${_out_var} "" PARENT_SCOPE)
endfunction()

function(fossredder_configure_vcpkg)
    if(NOT DEFINED VCPKG_TARGET_TRIPLET OR "${VCPKG_TARGET_TRIPLET}" STREQUAL "")
        set(VCPKG_TARGET_TRIPLET "x64-windows" CACHE STRING "vcpkg target triplet")
    endif()

    fossredder_read_windows_env_registry("VCPKG_ROOT" _vcpkg_root_from_registry)
    fossredder_read_windows_env_registry("VCPKG_INSTALLED_DIR" _vcpkg_installed_dir_from_registry)

    if(NOT "${_vcpkg_installed_dir_from_registry}" STREQUAL "")
        get_filename_component(_vcpkg_installed_parent_dir "${_vcpkg_installed_dir_from_registry}" DIRECTORY)
        get_filename_component(_vcpkg_installed_parent_name "${_vcpkg_installed_parent_dir}" NAME)
        if(_vcpkg_installed_parent_name STREQUAL "installed")
            get_filename_component(_vcpkg_root_from_installed_dir "${_vcpkg_installed_parent_dir}" DIRECTORY)
        endif()
    endif()

    if(NOT "${_vcpkg_root_from_registry}" STREQUAL "")
        set(_vcpkg_root "${_vcpkg_root_from_registry}")
    elseif(DEFINED _vcpkg_root_from_installed_dir AND NOT "${_vcpkg_root_from_installed_dir}" STREQUAL "")
        set(_vcpkg_root "${_vcpkg_root_from_installed_dir}")
    elseif(DEFINED ENV{VCPKG_ROOT} AND NOT "$ENV{VCPKG_ROOT}" STREQUAL "")
        file(TO_CMAKE_PATH "$ENV{VCPKG_ROOT}" _vcpkg_root)
    endif()

    if(NOT DEFINED CMAKE_TOOLCHAIN_FILE OR "${CMAKE_TOOLCHAIN_FILE}" STREQUAL "")
        if(DEFINED _vcpkg_root AND NOT "${_vcpkg_root}" STREQUAL "")
            set(CMAKE_TOOLCHAIN_FILE "${_vcpkg_root}/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "vcpkg toolchain file")
        else()
            message(FATAL_ERROR
                "CMAKE_TOOLCHAIN_FILE is not set and VCPKG_ROOT is unavailable. Set VCPKG_ROOT once on this machine or configure with -D CMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake."
            )
        endif()
    endif()

    if(NOT EXISTS "${CMAKE_TOOLCHAIN_FILE}")
        message(FATAL_ERROR
            "CMAKE_TOOLCHAIN_FILE points to '${CMAKE_TOOLCHAIN_FILE}', but the file does not exist. Install vcpkg, set VCPKG_ROOT once on this machine and reconfigure using the documented workflow."
        )
    endif()

    if((NOT DEFINED _vcpkg_root OR "${_vcpkg_root}" STREQUAL "") AND DEFINED CMAKE_TOOLCHAIN_FILE AND NOT "${CMAKE_TOOLCHAIN_FILE}" STREQUAL "")
        get_filename_component(_vcpkg_buildsystems_dir "${CMAKE_TOOLCHAIN_FILE}" DIRECTORY)
        get_filename_component(_vcpkg_scripts_dir "${_vcpkg_buildsystems_dir}" DIRECTORY)
        get_filename_component(_vcpkg_root "${_vcpkg_scripts_dir}" DIRECTORY)
    endif()

    if(NOT DEFINED VCPKG_INSTALLED_DIR OR "${VCPKG_INSTALLED_DIR}" STREQUAL "")
        if(NOT "${_vcpkg_installed_dir_from_registry}" STREQUAL "")
            set(VCPKG_INSTALLED_DIR "${_vcpkg_installed_dir_from_registry}" CACHE PATH "vcpkg installed dir")
        elseif(DEFINED ENV{VCPKG_INSTALLED_DIR} AND NOT "$ENV{VCPKG_INSTALLED_DIR}" STREQUAL "")
            file(TO_CMAKE_PATH "$ENV{VCPKG_INSTALLED_DIR}" _vcpkg_installed_dir)
            set(VCPKG_INSTALLED_DIR "${_vcpkg_installed_dir}" CACHE PATH "vcpkg installed dir")
        elseif(DEFINED _vcpkg_root AND NOT "${_vcpkg_root}" STREQUAL "")
            set(VCPKG_INSTALLED_DIR "${_vcpkg_root}/installed/fossredder" CACHE PATH "vcpkg installed dir")
        else()
            message(FATAL_ERROR
                "VCPKG_INSTALLED_DIR is not set and no vcpkg root could be derived. Set VCPKG_ROOT once on this machine, set VCPKG_INSTALLED_DIR explicitly, or configure with a vcpkg toolchain file path."
            )
        endif()
    endif()
endfunction()
