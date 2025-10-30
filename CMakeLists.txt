cmake_minimum_required(VERSION 3.30...4.0)

project(incplot)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  add_compile_options(/utf-8)
endif()

set(INCPLOT_VERSION_MAJOR 0)
set(INCPLOT_VERSION_MINOR 4)
set(INCPLOT_VERSION_PATCH 3)


########################################################
### LTO - try to enable for all dependent targets ###
########################################################
include(CheckIPOSupported)
check_ipo_supported(RESULT result OUTPUT output)

# Enable LTO if supported and only when NOT in debug mode
if(result)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
    message(STATUS "LTO is disabled because of Debug build")
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
    message(STATUS "LTO is disabled on MacOS")
  else()
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    message(STATUS "LTO is enabled")
    # set_property(TARGET foo PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()

else()
  message(WARNING "LTO is not supported: ${output}")
endif()


########################################################
### Target specification ###
########################################################
add_executable(incplot)
target_sources(incplot PRIVATE src/main.cpp src/config.cpp src/args.cpp)
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

include(CMake_dependencies.cmake) ### Loads and declares the required external libraries using CMake's FetchContent.
target_link_libraries(incplot PRIVATE incplot-lib::incplot-lib argparse::argparse "-lstdc++exp")
target_link_libraries(incplot PRIVATE sqlpp23::sqlpp23 sqlpp23::sqlite3)


#############################################################
### Automatic code generation (for version numbers, etc.) ###
#############################################################
set(COM_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/auto/versiondef.h)

add_custom_command(
  OUTPUT ${COM_OUTPUT}
  COMMAND ${CMAKE_COMMAND} -D TOP_DIR=${CMAKE_CURRENT_SOURCE_DIR} -D VER_MAJOR=${INCPLOT_VERSION_MAJOR} -D VER_MINOR=${INCPLOT_VERSION_MINOR} -D VER_PATCH=${INCPLOT_VERSION_PATCH} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/versionUpdate.cmake
  DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
  VERBATIM)

add_custom_target(ENSUREAutoFileDeps ALL DEPENDS ${COM_OUTPUT})
add_dependencies(incplot ENSUREAutoFileDeps)


########################################################
### Install specification (mostly relevant to CPack) ###
########################################################
install(TARGETS incplot RUNTIME DESTINATION .
  LIBRARY DESTINATION .)

install(FILES README.md LICENSE.txt DESTINATION .)
install(FILES data/configDB.sqlite DESTINATION .)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  add_subdirectory(cmake/packaging)
endif()
