# detect_stdlib.cmake

get_property(__STDLIB_ALREADY_RUN GLOBAL PROPERTY STDLIB_DETECTED SET)
if(__STDLIB_ALREADY_RUN)
    unset(__STDLIB_ALREADY_RUN)
    return()
endif()


include(CheckCXXSourceCompiles)

# libc++
check_cxx_source_compiles("
#include <cstddef>
#ifdef _LIBCPP_VERSION
int main() { return 0; }
#else
#error
#endif
"    USING_LIBCXX)

# libstdc++
check_cxx_source_compiles("
#include <cstddef>
#ifdef _GLIBCXX_RELEASE
int main() { return 0; }
#else
#error
#endif
"    USING_LIBSTDCXX)

# MSVC STL
check_cxx_source_compiles("
#include <yvals_core.h>
#ifdef _MSVC_STL_VERSION
int main() { return 0; }
#else
#error
#endif
"    USING_MSVC_STL)


# ---- Validation: exactly one must be true ----
set(__STDLIB_COUNT 0)
if(USING_LIBCXX)
    math(EXPR __STDLIB_COUNT "${__STDLIB_COUNT}+1")
endif()
if(USING_LIBSTDCXX)
    math(EXPR __STDLIB_COUNT "${__STDLIB_COUNT}+1")
endif()
if(USING_MSVC_STL)
    math(EXPR __STDLIB_COUNT "${__STDLIB_COUNT}+1")
endif()

if(__STDLIB_COUNT EQUAL 0)
    message(FATAL_ERROR
        "Failed to detect the C++ standard library.\n"
        "None of libc++, libstdc++, or MSVC STL were detected.\n"
        "Compiler: ${CMAKE_CXX_COMPILER}\n"
        "Compiler frontend variant: ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}\n"
        "Please ensure a supported standard library is available."
    )
elseif(__STDLIB_COUNT GREATER 1)
    message(FATAL_ERROR
        "Multiple C++ standard libraries detected (this should never ever happen).\n"
        "Detected:\n"
        "  libc++      = ${USING_LIBCXX}\n"
        "  libstdc++   = ${USING_LIBSTDCXX}\n"
        "  MSVC STL   = ${USING_MSVC_STL}\n"
        "Please report this configuration as a bug."
    )
endif()

set_property(GLOBAL PROPERTY STDLIB_DETECTED TRUE)
unset(__STDLIB_COUNT)
unset(__STDLIB_ALREADY_RUN)
