include(cmake/CPM.cmake)

if(BUILD_SHARED_LIBS)
    set(CPM_USE_LOCAL_PACKAGES ON)
endif()

CPMAddPackage("gh:InCom-0/incplot-lib#main")
CPMAddPackage("gh:p-ranav/argparse@3.2")

CPMAddPackage(
    URI "gh:InCom-0/sqlite3-cmake#master"
    OPTIONS "BUILD_SHARED_LIBS OFF"
    NAME SQLite3
)

set(BUILD_SQLITE3_CONNECTOR ON)
CPMAddPackage("gh:rbock/sqlpp23#0.67")
