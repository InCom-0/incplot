include(cmake/CPM.cmake)

CPMAddPackage("gh:InCom-0/incplot-lib#main")
CPMAddPackage("gh:p-ranav/argparse@3.2")
CPMAddPackage(
    URI "gh:sjinks/sqlite3-cmake#update-sqlite-3.51.0"
    OPTIONS "sqlite3_BUILD_SHARED_LIBS OFF"
    NAME SQLite3
)

set(BUILD_SQLITE3_CONNECTOR ON)
CPMAddPackage("gh:rbock/sqlpp23#0.67")