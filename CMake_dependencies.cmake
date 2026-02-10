if(NOT DEFINED CPM_USE_LOCAL_PACKAGES)
    set(CPM_USE_LOCAL_PACKAGES ${incplot_USE_LOCAL_PACKAGES} CACHE BOOL "CPM will try to find packages locally first" FORCE)
endif()
if(NOT DEFINED CPM_LOCAL_PACKAGES_ONLY)
    set(CPM_LOCAL_PACKAGES_ONLY ${incplot_USE_LOCAL_PACKAGES_ONLY} CACHE BOOL
        "CPM will not be forbidden from downloading packages. Will have to use local packages." FORCE)
endif()

# TODO: Need to fix this logic somehow
if(USING_MSVC_STL OR (MINGW AND (CMAKE_BUILD_TYPE STREQUAL "Release")))
    set(CPM_USE_LOCAL_PACKAGES OFF CACHE BOOL "Forbid finding local packages" FORCE)
endif()

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
if(NOT DEFINED CURL_BROTLI)
    set(CURL_BROTLI OFF CACHE BOOL "Enable brotli in curl")
endif()
if(NOT DEFINED CURL_ZSTD)
    set(CURL_ZSTD OFF CACHE BOOL "Enable zstd in curl")
endif()
if(NOT DEFINED USE_NGHTTP2)
    set(USE_NGHTTP2 OFF CACHE BOOL "Enable nghttp2 in curl")
endif()
if(NOT DEFINED CURL_USE_LIBSSH2)
    set(CURL_USE_LIBSSH2 OFF CACHE BOOL "Enable libssh2 in curl")
endif()
if((CMAKE_CXX_COMPILER_FRONTEND_VARIANT MATCHES "MSVC") OR (MINGW AND (CMAKE_BUILD_TYPE STREQUAL "Release")))
    if(NOT DEFINED USE_WIN32_IDN)
        # Using LIBIDN2 is impossible on MSVC and undesirable with MinGW Release, because it depends on a ton of other libs 
        set(USE_WIN32_IDN ON CACHE BOOL "Force usa of WIN32_IDN on MSVC and on MinGW when 'Release'")
    endif()
endif()


CPMAddPackage(
    URI "gh:libcpr/cpr#1.14.1"
    OPTIONS "BUILD_SHARED_LIBS OFF" "CPR_CURL_USE_LIBPSL OFF"
    NAME cpr
)

CPMAddPackage(
    URI "gh:InCom-0/libarchive_superbuild#main"
    OPTIONS
    # "BUILD_SHARED_LIBS OFF"
    "libarchive_sb_USE_LOCAL_PACKAGES ${incplot_USE_LOCAL_PACKAGES}"
    "ENABLE_MBEDTLS OFF"
    "ENABLE_LZMA ON"
    "ENABLE_ZLIB OFF"
    "ENABLE_BZip2 OFF"
    "ENABLE_LIBB2 OFF"
    "ENABLE_LZ4 OFF"
    "ENABLE_ZSTD OFF"
    "ENABLE_EXPAT OFF"
    "ENABLE_ICONV OFF"
    "ENABLE_PCREPOSIX OFF"
    "ENABLE_PCRE2POSIX OFF"
    "libarchive_sb_ENABLE_TEST OFF"
    "libarchive_sb_ENABLE_COVERAGE OFF"
    "libarchive_sb_ENABLE_INSTALL OFF"
    NAME libarchive_superbuild
)
