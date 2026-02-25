cmake_minimum_required(VERSION 3.30...4.2)

project(incplot
    HOMEPAGE_URL "https://github.com/InCom-0/incplot"
    VERSION "0.5.0"
    DESCRIPTION
    "CLI tool for drawing great looking plots in the terminal focused on user ergonomics and simplicity."
    LANGUAGES CXX)

set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(incplot_STAGE_OUTPUTS "Stage build output artifacts into proper directory structure" ON)

option(CPM_USE_LOCAL_PACKAGES "Use local (usually system installed) packages for dependencies." ON)
option(CPM_LOCAL_PACKAGES_ONLY "Forbid the build system from downloading missing dependencies." OFF)

option(incplot_CPR_USE_SYSTEM_CURL "Incplot transitively needs libcurl. Set to ON if you have and want to use system installed one" ON)


#####################################################################
### Staging ###
#####################################################################
include(cmake/StageOutputs.cmake)
include(cmake/MingwRuntimeDeps.cmake)
if(incplot_STAGE_OUTPUTS)
    stageOutputDirsFor(incplot)
endif()


#####################################################################
### Platform specifics + hacks + LTO ###
#####################################################################
if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT MATCHES "MSVC")
    add_compile_options(/utf-8)
    add_compile_definitions(NOMINMAX)
endif()

### This attempts to compile sample program to find out if we are using libc++(USING_LIBCXX) or libstdc++(USING_LIBSTDCXX) or MSVC_STL(USING_MSVC_STL)
include(cmake/stdlibQuery.cmake)

include(CheckIPOSupported)
check_ipo_supported(RESULT INCPLOT_LTO OUTPUT INCPLOT_LTO_OUTPUT)

# Enable LTO if supported and only when NOT in debug mode
if(INCPLOT_LTO)
    message(STATUS "LTO is supported")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
        message(STATUS "LTO is disabled because of Debug build")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
        message(STATUS "LTO is disabled on MacOS")
    else()
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
        message(STATUS "LTO is enabled")
        # set_property(TARGET foo PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()

else()
    message(WARNING "LTO is NOT supported: ${INCPLOT_LTO_OUTPUT}")
endif()


######################################################################
### Target specification ###
######################################################################
add_executable(incplot)
target_sources(incplot PRIVATE src/main.cpp src/config.cpp src/args.cpp src/uri.cpp)
target_sources(incplot
    PRIVATE
    FILE_SET priv_headers
    TYPE HEADERS
    BASE_DIRS
    include)

target_sources(incplot
    PRIVATE
    FILE_SET priv_auto_config
    TYPE HEADERS
    BASE_DIRS
    cmake/config)

target_compile_features(incplot PRIVATE cxx_std_23)

# TODO: Need to finish the script that drives this
# if(MINGW)
#     add_mingw_runtime_deps(incplot)
# endif(MINGW)


##########################################################################
### Settings the right options for linking for Windows and Apple  ###
##########################################################################
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(CMAKE_SYSTEM_NAME MATCHES "Windows")
        set(BUILD_SHARED_LIBS OFF)
        if(USING_LIBSTDCXX)
            target_link_options(incplot PUBLIC -static)
        elseif(USING_LIBCXX)
            target_link_options(incplot PUBLIC -stdlib=libc++ -static)
        elseif(USING_MSVC_STL)
        endif()

    elseif(APPLE)
        if(USING_LIBSTDCXX)
            target_link_options(incplot PUBLIC -static)
        elseif(USING_LIBCXX)
            target_link_options(incplot PUBLIC -stdlib=libc++)
        elseif(USING_MSVC_STL)
            message(FATAL_ERROR "CMake reports we are using MSVC STL on APPLE platform. That is impossible.")
        endif()
    else()
    endif()

elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
endif()


##########################################################################
### Dependencies and library linking  ###
##########################################################################
include(CMake_dependencies.cmake) ### Loads and declares the required external libraries using CPM or CMake's FetchContent.

target_link_libraries(incplot PRIVATE incplot-lib::incplot-lib incfontdisc::incfontdisc)
target_link_libraries(incplot PRIVATE sqlpp23::sqlpp23 sqlpp23::sqlite3)
target_link_libraries(incplot PRIVATE argparse::argparse cpr::cpr indicators::indicators)
target_link_libraries(incplot PRIVATE LibArchive::LibArchive)
target_link_libraries(incplot PRIVATE ots::ots)

if(USING_LIBSTDCXX)
    target_link_libraries(incplot PRIVATE "-lstdc++exp")
endif()

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(incplot PRIVATE LIBARCHIVE_STATIC)
endif()

# TODO: Figure out how to silence that warning on MinGW ... does not work like it is below
# if(SQLite3_ADDED)
#     set(sqlite3c_FILE "${CMAKE_BINARY_DIR}/_deps/sqlite3_ext-src/sqlite3.c")
#     set_source_files_properties(
#         ${sqlite3c_FILE}
#         PROPERTIES
#         COMPILE_OPTIONS "$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wno-stringop-overread>"
#     )
#     unset(sqlite3c_FILE)

# endif()



#############################################################
### Automatic copying certain assets to build directory   ###
#############################################################
add_custom_command(TARGET incplot POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Copying SQLite ConfigDB and JetBrainsMonoNerdFont to output directory"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/data/JetBrainsMonoNerdFont-Regular.ttf"
    "${CMAKE_SOURCE_DIR}/data/configDB.sqlite"
    "$<TARGET_FILE_DIR:incplot>"
)

#############################################################
### Automatic code generation (for version numbers, etc.) ###
#############################################################
set(COM_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/auto/versiondef.h)

add_custom_command(
    OUTPUT ${COM_OUTPUT}
    COMMAND ${CMAKE_COMMAND} -D TOP_DIR=${CMAKE_CURRENT_SOURCE_DIR} -D VER_MAJOR=${PROJECT_VERSION_MAJOR} -D VER_MINOR=${PROJECT_VERSION_MINOR} -D VER_PATCH=${PROJECT_VERSION_PATCH} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/versionUpdate.cmake
    DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
    VERBATIM)

add_custom_target(incplot_autoFileDeps ALL DEPENDS ${COM_OUTPUT})
add_dependencies(incplot incplot_autoFileDeps)


########################################################
### Install specification ###
########################################################
install(TARGETS incplot
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# install(TARGETS incplot RUNTIME DESTINATION .
#   LIBRARY DESTINATION .)

install(FILES README.md LICENSE.txt DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES data/configDB.sqlite DESTINATION ${CMAKE_INSTALL_BINDIR})

if(PROJECT_IS_TOP_LEVEL)
    add_subdirectory(cmake/packaging)
endif()
