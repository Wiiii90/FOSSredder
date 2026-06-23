include_guard(GLOBAL)

option(FOSSREDDER_ENABLE_CLANG_TIDY "Run clang-tidy for first-party C++ targets" OFF)
option(FOSSREDDER_ENABLE_COVERAGE "Enable LLVM source-based coverage instrumentation for first-party targets" OFF)

set(FOSSREDDER_CLANG_TIDY_HEADER_FILTER
    "^${CMAKE_SOURCE_DIR}/(app|core|diagnostics|infra|persistence|ui)/"
    CACHE STRING "Header filter used for clang-tidy on first-party code"
)

function(fossredder_configure_quality_tools)
    if(FOSSREDDER_ENABLE_CLANG_TIDY)
        find_program(FOSSREDDER_CLANG_TIDY_EXECUTABLE NAMES clang-tidy clang-tidy.exe REQUIRED)

        set(_clang_tidy_command
            "${FOSSREDDER_CLANG_TIDY_EXECUTABLE}"
            "--config-file=${CMAKE_SOURCE_DIR}/.clang-tidy"
            "--header-filter=${FOSSREDDER_CLANG_TIDY_HEADER_FILTER}"
        )

        set_property(GLOBAL PROPERTY FOSSREDDER_CLANG_TIDY_COMMAND "${_clang_tidy_command}")
        message(STATUS "clang-tidy enabled for first-party targets")
    endif()

    if(FOSSREDDER_ENABLE_COVERAGE)
        if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            message(FATAL_ERROR "FOSSREDDER_ENABLE_COVERAGE requires Clang or clang-cl. Use the coverage preset.")
        endif()

        if(MSVC)
            execute_process(
                COMMAND "${CMAKE_CXX_COMPILER}" --print-resource-dir
                OUTPUT_VARIABLE _clang_resource_dir
                OUTPUT_STRIP_TRAILING_WHITESPACE
                COMMAND_ERROR_IS_FATAL ANY
            )

            if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                set(_coverage_runtime_library "${_clang_resource_dir}/lib/windows/clang_rt.profile-x86_64.lib")
            else()
                set(_coverage_runtime_library "${_clang_resource_dir}/lib/windows/clang_rt.profile-i386.lib")
            endif()

            if(NOT EXISTS "${_coverage_runtime_library}")
                message(FATAL_ERROR "LLVM profile runtime library not found: ${_coverage_runtime_library}")
            endif()

            set(_coverage_compile_options
                /Z7
                /clang:-fprofile-instr-generate
                /clang:-fcoverage-mapping
            )
            set(_coverage_link_options
                /DEBUG:FULL
                /INCREMENTAL:NO
            )
            set(_coverage_compile_definitions
                FMT_USE_CONSTEVAL=0
                SPDLOG_NO_EXCEPTIONS=1
            )
        else()
            set(_coverage_compile_options
                -gline-tables-only
                -fprofile-instr-generate
                -fcoverage-mapping
            )
            set(_coverage_link_options
                -fprofile-instr-generate
                -fcoverage-mapping
            )
            set(_coverage_runtime_library)
            set(_coverage_compile_definitions)
        endif()

        set_property(GLOBAL PROPERTY FOSSREDDER_COVERAGE_COMPILE_OPTIONS "${_coverage_compile_options}")
        set_property(GLOBAL PROPERTY FOSSREDDER_COVERAGE_LINK_OPTIONS "${_coverage_link_options}")
        set_property(GLOBAL PROPERTY FOSSREDDER_COVERAGE_RUNTIME_LIBRARY "${_coverage_runtime_library}")
        set_property(GLOBAL PROPERTY FOSSREDDER_COVERAGE_COMPILE_DEFINITIONS "${_coverage_compile_definitions}")
        message(STATUS "LLVM coverage enabled for first-party targets")
    endif()
endfunction()

function(fossredder_enable_target_quality _target)
    if(NOT TARGET ${_target})
        message(FATAL_ERROR "Target '${_target}' does not exist for quality configuration")
    endif()

    get_target_property(_target_type ${_target} TYPE)
    if(_target_type STREQUAL "INTERFACE_LIBRARY")
        return()
    endif()

    if(FOSSREDDER_ENABLE_CLANG_TIDY)
        get_property(_clang_tidy_command GLOBAL PROPERTY FOSSREDDER_CLANG_TIDY_COMMAND)
        if(_clang_tidy_command)
            set_property(TARGET ${_target} PROPERTY CXX_CLANG_TIDY "${_clang_tidy_command}")
        endif()
    endif()

    if(FOSSREDDER_ENABLE_COVERAGE)
        get_property(_coverage_compile_options GLOBAL PROPERTY FOSSREDDER_COVERAGE_COMPILE_OPTIONS)
        get_property(_coverage_link_options GLOBAL PROPERTY FOSSREDDER_COVERAGE_LINK_OPTIONS)
        get_property(_coverage_runtime_library GLOBAL PROPERTY FOSSREDDER_COVERAGE_RUNTIME_LIBRARY)
        get_property(_coverage_compile_definitions GLOBAL PROPERTY FOSSREDDER_COVERAGE_COMPILE_DEFINITIONS)

        if(_coverage_compile_options)
            target_compile_options(${_target} PRIVATE ${_coverage_compile_options})
        endif()

        if(_coverage_compile_definitions)
            target_compile_definitions(${_target} PRIVATE ${_coverage_compile_definitions})
        endif()

        if(_coverage_link_options AND NOT _target_type STREQUAL "OBJECT_LIBRARY")
            target_link_options(${_target} PRIVATE ${_coverage_link_options})
        endif()

        if(_coverage_runtime_library AND (_target_type STREQUAL "EXECUTABLE" OR _target_type STREQUAL "SHARED_LIBRARY" OR _target_type STREQUAL "MODULE_LIBRARY"))
            target_link_libraries(${_target} PRIVATE "${_coverage_runtime_library}")
        endif()
    endif()
endfunction()
