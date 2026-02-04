set(CPM_USE_LOCAL_PACKAGES ${incplot_USE_LOCAL_PACKAGES} CACHE BOOL "CPM will try to find packages locally first" FORCE)
set(CPM_LOCAL_PACKAGES_ONLY ${incplot_USE_LOCAL_PACKAGES_ONLY} CACHE BOOL
    "CPM will not be forbidden from downloading packages. Will have to use local packages." FORCE)
    
include(cmake/CPM.cmake)


CPMAddPackage(
    NAME nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    URL_HASH SHA256=42f6e95cad6ec532fd372391373363b62a14af6d771056dbfc86160e6dfff7aa
    EXCLUDE_FROM_ALL TRUE
)

CPMAddPackage(
    URI "gh:InCom-0/incplot-lib#main"
    OPTIONS "incplot-lib_BUILD_SHARED_LIB OFF"
    NAME incplot-lib
)
CPMAddPackage(
    URI "gh:InCom-0/incfontdisc#main"
    OPTIONS "incfontdisc_BUILD_SHARED_LIB OFF"
    NAME incfontdisc
)
# CPMAddPackage(
#     URI "gh:InCom-0/ots_cmake#cmake_unofficial"
#     OPTIONS "ots_BUILD_SHARED_LIB OFF"
#     NAME ots
# )
CPMAddPackage(
    URI "gh:p-ranav/argparse@3.2"
)
CPMAddPackage(
    URI "gh:p-ranav/indicators@2.3"
)

CPMAddPackage(
    URI "gh:InCom-0/sqlite3-cmake#master"
    OPTIONS "BUILD_SHARED_LIBS OFF"
    NAME SQLite3
)
set(BUILD_SQLITE3_CONNECTOR ON)
CPMAddPackage("gh:rbock/sqlpp23#0.67")

# cpr is a library that wraps curl in a sane way, but also builds its dependencies if need be or just links the system installed ones
# On a 'normal' non-windows system it is probably better to have CURL already installed
# Same goes for LIBPSL
if(NOT DEFINED CURL_ZLIB)
    set(CURL_ZLIB OFF CACHE BOOL "Enable zlib in curl")
endif()
CPMAddPackage(
    URI "gh:libcpr/cpr#1.14.1"
    OPTIONS "BUILD_SHARED_LIBS OFF"
    NAME cpr
)

CPMAddPackage(
    URI "gh:tukaani-project/xz@5.8.2"
    OPTIONS "BUILD_SHARED_LIBS OFF"
    NAME LibLZMA
)

CPMAddPackage(
    URI "gh:libarchive/libarchive@3.8.5"
    OPTIONS
    "BUILD_SHARED_LIBS OFF"
    "ENABLE_ZLIB OFF"
    "ENABLE_BZip2 OFF"
    "ENABLE_LIBB2 OFF"
    "ENABLE_LZ4 OFF"
    "ENABLE_ZSTD OFF"
    "ENABLE_EXPAT OFF"
    # "ENABLE_LIBGCC OFF"
    # "ENABLE_PCREPOSIX OFF"
    # "ENABLE_PCRE2POSIX OFF"
    "LIBLZMA_INCLUDE_DIRS ${LibLZMA_SOURCE_DIR}/src/liblzma/api"
    "LIBLZMA_LIBRARIES liblzma"
    "ENABLE_TEST OFF"
    "ENABLE_INSTALL OFF"
    NAME LibArchive
)

if(LibArchive_ADDED)
    if(NOT TARGET LibArchive::LibArchive)
        add_library(LibArchive::LibArchive ALIAS archive_static)
    endif()
endif()
