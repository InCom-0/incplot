cmake_minimum_required(VERSION 3.30...4.0)

project(incplot
  HOMEPAGE_URL "https://github.com/InCom-0/incplot"
  VERSION "0.5.0"
  DESCRIPTION
  "CLI tool for drawing great looking plots in the terminal focused on user ergonomics and simplicity."
  LANGUAGES CXX)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)


#####################################################################
### Platform specifics + hacks + LTO ###
#####################################################################
if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT MATCHES "MSVC")
  add_compile_options(/utf-8)
  add_compile_definitions(NOMINMAX)
endif()

### This attempts to compile sample program to find out if we are using libc++(USING_LIBCXX) or libstdc++(USING_LIBSTDCXX) or MSVC_STL(USING_MSVC_STL)
include(cmake/stdlibQuery.cmake)
if(USING_LIBSTDCXX)
  message(STATUS "Using libstdc++")
elseif(USING_LIBCXX)
  message(STATUS "Using libc++")
elseif(USING_MSVC_STL)
  message(STATUS "Using MSVC STL")
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT INCPLOT_LTO OUTPUT INCPLOT_LTO_OUTPUT)

# Enable LTO if supported and only when NOT in debug mode
if(INCPLOT_LTO)
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
  message(WARNING "LTO is not supported: ${INCPLOT_LTO_OUTPUT}")
endif()


######################################################################
### Target specification ###
######################################################################
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


##########################################################################
### Settings the right options for linking for Windows and Apple  ###
##########################################################################
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(BUILD_SHARED_LIBS OFF)
    if(USING_LIBSTDCXX)
      target_link_options(incplot PUBLIC -static)
    elseif(USING_LIBCXX)
      target_link_options(incplot PUBLIC -stdlib=libc++ -static)
    elseif(USING_MSVC_STL)
    endif()

  elseif(APPLE)
    if(USING_LIBSTDCXX)
      target_link_options(incplot PUBLIC -static)
    elseif(USING_LIBCXX)
      target_link_options(incplot PUBLIC -stdlib=libc++)
    endif()
  endif()

elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(BUILD_SHARED_LIBS ON)
endif()


##########################################################################
### Dependencies and library linking  ###
##########################################################################
include(CMake_dependencies.cmake) ### Loads and declares the required external libraries using CPM or CMake's FetchContent.

target_link_libraries(incplot PRIVATE incplot-lib::incplot-lib argparse::argparse)
target_link_libraries(incplot PRIVATE sqlpp23::sqlpp23 sqlpp23::sqlite3)

if(USING_LIBSTDCXX)
  target_link_libraries(incplot PRIVATE "-lstdc++exp")
endif()


#############################################################
### Automatic copying certain assets to build directory   ###
#############################################################
add_custom_command(TARGET incplot POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "Copying SQLite ConfigDB and JetBrainsMonoNerdFont to output directory"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  "${CMAKE_SOURCE_DIR}/data/JetBrainsMonoNerdFont-Regular.ttf"
  "${CMAKE_SOURCE_DIR}/data/configDB.sqlite"
  "$<TARGET_FILE_DIR:incplot>"
)

#############################################################
### Automatic code generation (for version numbers, etc.) ###
#############################################################
set(COM_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/auto/versiondef.h)

add_custom_command(
  OUTPUT ${COM_OUTPUT}
  COMMAND ${CMAKE_COMMAND} -D TOP_DIR=${CMAKE_CURRENT_SOURCE_DIR} -D VER_MAJOR=${PROJECT_VERSION_MAJOR} -D VER_MINOR=${PROJECT_VERSION_MINOR} -D VER_PATCH=${PROJECT_VERSION_PATCH} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/config/versionUpdate.cmake
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
