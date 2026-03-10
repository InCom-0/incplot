#=============================================================================
# MingwRuntimeDeps.cmake
#
# Adds a post-build step to copy missing MinGW runtime DLLs next to the target.
#=============================================================================

function(add_mingw_runtime_deps target_name)
    if(NOT MINGW)
        message(FATAL_ERROR "add_mingw_runtime_deps(): you are trying to add mingw runtime dependencies when not running mingw; this is impossible; please call add_mingw_runtime_deps() only conditionally when MINGW is defined")
        return()
    endif()

    if(NOT TARGET "${target_name}")
        message(FATAL_ERROR "add_mingw_runtime_deps(): target '${target_name}' does not exist")
    endif()

    if(CMAKE_CXX_COMPILER)
        set(_compiler_path "${CMAKE_CXX_COMPILER}")
    elseif(CMAKE_C_COMPILER)
        set(_compiler_path "${CMAKE_C_COMPILER}")
    else()
        set(_compiler_path "")
    endif()

    if("${_compiler_path}" STREQUAL "")
        message(FATAL_ERROR "add_mingw_runtime_deps(): no C or CXX compiler set; cannot locate MinGW runtime directory")
    endif()

    if(NOT "${_compiler_path}" STREQUAL "")
        get_filename_component(_compiler_dir "${_compiler_path}" DIRECTORY)
    else()
        set(_compiler_dir "")
    endif()
    get_filename_component(_compiler_root "${_compiler_dir}/../.." ABSOLUTE)
    set(_compiler_usr_bin "${_compiler_root}/usr/bin")

    find_program(MINGW_LDD
        NAMES ldd ldd.exe
        PATHS
        "${_compiler_usr_bin}"
        NO_DEFAULT_PATH
    )

    if(NOT MINGW_LDD)
        message(WARNING "add_mingw_runtime_deps(): ldd.exe not found in ${_compiler_usr_bin}; skipping runtime dependency copying")
        return()
    endif()

    add_custom_command(TARGET "${target_name}" POST_BUILD
        COMMAND "${CMAKE_COMMAND}"
        -D MINGW_RUNTIME_DEPS_SCRIPT_MODE=ON
        -D TARGET_FILE=$<TARGET_FILE:${target_name}>
        -D RUNTIME_OUTPUT_DIR=$<TARGET_FILE_DIR:${target_name}>
        -D COMPILER_DIR=${_compiler_dir}
        -D LDD_PATH=${MINGW_LDD}
        -D BUILD_CONFIG=$<CONFIG>
        -D DEFAULT_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -P ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/MingwRuntimeDeps.cmake
        VERBATIM
    )
endfunction()

if(MINGW_RUNTIME_DEPS_SCRIPT_MODE)
    message(STATUS "Mingw runtime deps: TARGET_FILE='${TARGET_FILE}', RUNTIME_OUTPUT_DIR='${RUNTIME_OUTPUT_DIR}', CONFIG='${BUILD_CONFIG}'")

    if(NOT DEFINED TARGET_FILE OR NOT EXISTS "${TARGET_FILE}")
        message(STATUS "Mingw runtime deps: target file missing, skipping")
        return()
    endif()

    if(NOT DEFINED RUNTIME_OUTPUT_DIR OR "${RUNTIME_OUTPUT_DIR}" STREQUAL "")
        message(STATUS "Mingw runtime deps: output directory missing, skipping")
        return()
    endif()

    set(_config "${BUILD_CONFIG}")
    if("${_config}" STREQUAL "")
        set(_config "${DEFAULT_BUILD_TYPE}")
    endif()

    string(TOUPPER "${_config}" _config_uc)
    if(NOT _config_uc STREQUAL "RELEASE")
        message(STATUS "Mingw runtime deps: CMAKE_BUILD_TYPE is not 'Release', skipping")
        return()
    endif()

    if(NOT DEFINED COMPILER_DIR OR NOT IS_DIRECTORY "${COMPILER_DIR}")
        message(STATUS "Mingw runtime deps: compiler directory missing, skipping")
        return()
    endif()

    if(NOT DEFINED LDD_PATH OR NOT EXISTS "${LDD_PATH}")
        message(STATUS "Mingw runtime deps: ldd.exe missing, skipping")
        return()
    endif()

    get_filename_component(_ldd_dir "${LDD_PATH}" DIRECTORY)
    set(_clean_path "${_ldd_dir};C:/Windows/System32;C:/Windows")

    file(MAKE_DIRECTORY "${RUNTIME_OUTPUT_DIR}")

    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E env "PATH=${_clean_path}" "${LDD_PATH}" "${TARGET_FILE}"
        OUTPUT_VARIABLE _ldd_out
        ERROR_VARIABLE _ldd_err
        RESULT_VARIABLE _ldd_result
    )

    if(NOT _ldd_result EQUAL 0)
        message(STATUS "Mingw runtime deps: ldd failed: ${_ldd_result}")
        if(NOT "${_ldd_err}" STREQUAL "")
            message(STATUS "Mingw runtime deps: ldd stderr: ${_ldd_err}")
        endif()
        return()
    endif()

    string(REPLACE "\r" "" _ldd_out "${_ldd_out}")
    string(REPLACE "\n" ";" _ldd_lines "${_ldd_out}")

    set(_missing_dlls "")
    foreach(_line IN LISTS _ldd_lines)
        # Ignore entries resolved via a fallback that end with "(?)"
        if(_line MATCHES "\\(\\?\\)[ \t]*$")
            continue()
        endif()

        if(_line MATCHES "^[ \t]*([^ \t]+)[ \t]*=>[ \t]*not[ \t]+found[ \t]*$")
            set(_dll "${CMAKE_MATCH_1}")
            string(STRIP "${_dll}" _dll)
            list(APPEND _missing_dlls "${_dll}")
        endif()
    endforeach()

    if(_missing_dlls)
        list(REMOVE_DUPLICATES _missing_dlls)
    endif()

    foreach(_dll IN LISTS _missing_dlls)
        set(_src "${COMPILER_DIR}/${_dll}")
        set(_dst "${RUNTIME_OUTPUT_DIR}/${_dll}")

        message(STATUS "_src: ${_src}")
        message(STATUS "_dst ${_dst}")

        if(EXISTS "${_src}")
            file(COPY_FILE "${_src}" "${_dst}" ONLY_IF_DIFFERENT)
            message(STATUS "Mingw runtime deps: copied ${_dll}")
        else()
            message(STATUS "Mingw runtime deps: missing ${_dll} in ${COMPILER_DIR}")
        endif()
    endforeach()
endif()
