cmake_minimum_required(VERSION 3.30...4.0)

project(incplot)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

set(INCPLOT_VERSION_MAJOR 0)
set(INCPLOT_VERSION_MINOR 1)
set(INCPLOT_VERSION_PATCH 7)


add_executable(incplot)
target_sources(incplot PRIVATE src/main.cpp src/cross_platform.cpp)
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

add_subdirectory(sub/incplot-lib)
target_link_libraries(incplot PRIVATE incplot-lib::incplot-lib "-lstdc++exp")


set(COM_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/auto/versiondef.h)

add_custom_command(
  OUTPUT ${COM_OUTPUT}
  COMMAND ${CMAKE_COMMAND} -D TOP_DIR=${CMAKE_CURRENT_SOURCE_DIR} -D VER_MAJOR=${INCPLOT_VERSION_MAJOR} -D VER_MINOR=${INCPLOT_VERSION_MINOR} -D VER_PATCH=${INCPLOT_VERSION_PATCH} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/versionUpdate.cmake
  DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
  VERBATIM)

add_custom_target(ENSUREAutoFileDeps ALL DEPENDS ${COM_OUTPUT})
add_dependencies(incplot ENSUREAutoFileDeps)


install(TARGETS incplot RUNTIME DESTINATION bin
                        LIBRARY DESTINATION bin)

install(FILES README.md LICENSE.txt DESTINATION bin)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  add_subdirectory(cmake/packaging)
endif()