cmake_minimum_required(VERSION 3.30...4.0)

project(incplot)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)


add_executable(incplot)

list(TRANSFORM INCPLOT_LIB_SRC PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/src/)

target_sources(incplot PRIVATE src/main.cpp src/cross_platform.cpp)
target_sources(incplot
    PRIVATE
        FILE_SET pub_headers
        TYPE HEADERS
        BASE_DIRS
            include)

target_compile_features(incplot PUBLIC cxx_std_23)


include(CMake_dependencies.cmake) ### Loads and declares the requires external libraries using both vcpkg and CMake's FetchContent.
target_link_libraries(incplot PRIVATE incplot-lib "-lstdc++exp")