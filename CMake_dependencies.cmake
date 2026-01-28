include(cmake/CPM.cmake)

if(BUILD_SHARED_LIBS)
    set(CPM_USE_LOCAL_PACKAGES ON)
endif()

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

# cpr is a library that wraps curl in a sane way, but also builds its dependencies if need be or just links the system installed ones
# On a 'normal' non-windows system it is probably better to have CURL already installed
# Same goes for LIBPSL
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
