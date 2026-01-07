include(cmake/CPM.cmake)

CPMAddPackage("gh:InCom-0/incplot-lib#main")
CPMAddPackage("gh:p-ranav/argparse@3.2")

if(BUILD_SHARED_LIBS)
    set(CPM_USE_LOCAL_PACKAGES ON)
endif()

CPMAddPackage(
    URI "gh:InCom-0/sqlite3-cmake#master"
    NAME SQLite3
)

set(BUILD_SQLITE3_CONNECTOR ON)
CPMAddPackage("gh:rbock/sqlpp23#0.67")
