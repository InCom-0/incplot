include(FetchContent)

# FetchContent_Declare(
#     googletest
#     GIT_REPOSITORY https://github.com/google/googletest.git
#     GIT_TAG v1.16.0
# )
# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
    incplot-lib
    GIT_REPOSITORY https://github.com/InCom-0/incplot-lib
    GIT_TAG main
)
FetchContent_MakeAvailable(incplot-lib)