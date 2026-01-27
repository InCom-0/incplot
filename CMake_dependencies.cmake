include(cmake/CPM.cmake)

if(BUILD_SHARED_LIBS)
    set(CPM_USE_LOCAL_PACKAGES ON)
endif()

CPMAddPackage(
    URI "gh:InCom-0/incplot-lib#main"
    NAME incplot-lib
)
CPMAddPackage(
    URI "gh:InCom-0/incfontdisc#main"
    OPTIONS "incfontdisc_BUILD_SHARED_LIB OFF"
    NAME incfontdisc
)
CPMAddPackage(
    URI "gh:p-ranav/argparse@3.2"
)

CPMAddPackage(
    URI "gh:InCom-0/sqlite3-cmake#master"
    OPTIONS "BUILD_SHARED_LIBS OFF"
    NAME SQLite3
)
set(BUILD_SQLITE3_CONNECTOR ON)
CPMAddPackage("gh:rbock/sqlpp23#0.67")



CPMAddPackage(URI "gh:curl/curl#curl-8_18_0"
    NAME CURL
    OPTIONS
    "CURL_USE_LIBPSL OFF"
    "USE_LIBIDN2 OFF"
    "USE_NGHTTP2 OFF"
    "CURL_USE_LIBSSH2 OFF"

    "CURL_BROTLI OFF"
    "CURL_ZSTD OFF"
    "CURL_ZLIB OFF"
    "BUILD_CURL_EXE OFF"
    "CURL_DISABLE_INSTALL ON"
    "BUILD_SHARED_LIBS OFF"
    "BUILD_STATIC_LIBS ON"
    "ENABLE_UNICODE ON"
    "CURL_USE_SCHANNEL ON"
    "CURL_LTO ${INCPLOT_LTO}"
)


CPMAddPackage(
    URI "gh:tukaani-project/xz@5.8.2"
    OPTIONS "BUILD_SHARED_LIBS OFF"
    NAME LibLZMA
)
if(LibLZMA_ADDED)
    set(LIBLZMA_FOUND TRUE)
endif()

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
