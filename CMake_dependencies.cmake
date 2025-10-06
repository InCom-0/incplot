include(FetchContent)

FetchContent_Declare(
    incplot-lib
    GIT_REPOSITORY https://github.com/InCom-0/incplot-lib
    GIT_TAG main
)
FetchContent_MakeAvailable(incplot-lib)

# Get sqlite3
FetchContent_Declare(
    sqlite3
    GIT_REPOSITORY https://github.com/InCom-0/sqlite3-cmake
    GIT_TAG master
)
FetchContent_MakeAvailable(sqlite3)

# Write a fake package config so that subprojects find our SQLite that's defined by sqlite3-cmake
file(WRITE "${sqlite3_BINARY_DIR}/SQLite3Config.cmake" 
"if(NOT TARGET SQLite::SQLite3)
  add_library(SQLite::SQLite3 ALIAS sqlite3)
endif()")
set(SQLite3_DIR "${sqlite3_BINARY_DIR}")

# Now bring in sqlpp23
FetchContent_Declare(
    sqlpp23
    GIT_REPOSITORY  https://github.com/InCom-0/sqlpp23
    GIT_TAG         main
)
# Configure sqlpp23 for SQLite
# set(BUILD_MYSQL_CONNECTOR ON)
# set(BUILD_MARIADB_CONNECTOR ON)
# set(BUILD_POSTGRESQL_CONNECTOR ON)
set(BUILD_SQLITE3_CONNECTOR ON)
# set(BUILD_SQLCIPHER_CONNECTOR ON)

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)
FetchContent_MakeAvailable(sqlpp23)
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG OFF)