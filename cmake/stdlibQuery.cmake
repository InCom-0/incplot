include(CheckCXXSourceCompiles)

check_cxx_source_compiles("
#include <cstddef>
#ifdef _LIBCPP_VERSION
int main() { return 0; }
#else
#error
#endif
"    USING_LIBCXX)

check_cxx_source_compiles("
#include <cstddef>
#ifdef _GLIBCXX_RELEASE
int main() { return 0; }
#else
#error
#endif
"    USING_LIBSTDCXX)