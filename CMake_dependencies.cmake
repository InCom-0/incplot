include(cmake/CPM.cmake)

CPMAddPackage("gh:InCom-0/incplot-lib#main")
CPMAddPackage("gh:p-ranav/argparse#master")
CPMAddPackage("gh:InCom-0/sqlite3-cmake#master")
CPMAddPackage("gh:InCom-0/incsfntly#main")


##############################################################################
##### SQLite3 dance so that sqlpp23 finds our SQLite and not the system one 
##############################################################################
file(WRITE "${sqlite3_BINARY_DIR}/SQLite3Config.cmake"
    "if(NOT TARGET SQLite::SQLite3)
  add_library(SQLite::SQLite3 ALIAS sqlite3)
endif()")
set(SQLite3_DIR "${sqlite3_BINARY_DIR}")


##############################################################################
##### Adding sqlpp23 ... only building SQLITE connector  
##############################################################################
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)
set(BUILD_SQLITE3_CONNECTOR ON)
CPMAddPackage("gh:InCom-0/sqlpp23#main")
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG OFF)