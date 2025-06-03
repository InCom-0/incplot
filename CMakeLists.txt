cmake_minimum_required(VERSION 3.30...4.0)

project(incplot)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

set(INCPLOT_VERSION_MAJOR 0)
set(INCPLOT_VERSION_MINOR 1)
set(INCPLOT_VERSION_PATCH 0)


add_executable(incplot)
target_sources(incplot PRIVATE src/main.cpp src/cross_platform.cpp)
target_sources(incplot
    PRIVATE
        FILE_SET pub_headers
        TYPE HEADERS
        BASE_DIRS
            include)

target_compile_features(incplot PRIVATE cxx_std_23)

include(CMake_dependencies.cmake) ### Loads and declares the requires external libraries using both vcpkg and CMake's FetchContent.
target_link_libraries(incplot PRIVATE incplot-lib "-lstdc++exp")

install(TARGETS incplot RUNTIME DESTINATION bin
                        LIBRARY DESTINATION bin)

install(FILES README.md LICENSE.txt DESTINATION bin)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  add_subdirectory(cmake.packaging)
endif()